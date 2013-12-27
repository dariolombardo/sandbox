//
// Copyright 2013
// Dario Lombardo <lomato@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include <GeoIP.h>
#include <GeoIPCity.h>

#include <cassert>
#include <map>
#include <iostream>
#include <fstream>
#include <set>
#include <unistd.h>
#include <memory>
#include <algorithm>
#include <csignal>
#include <iomanip>

using namespace std;

typedef pair<float,float> GeoPair;

#define GEOIPFILE "/usr/share/GeoIP/GeoIPCity.dat"

unsigned totlines;
unsigned partial = 0;

void print_stats(int signo)
{
    cout << "\r" << (partial*100/totlines) << "%";
    cout.flush();
    alarm(1);
}

void usage(char* argv0)
{
    cout << "\nUsage: " << argv0 << " -i <infile> [-o <outfile>] [-f <filter>]" <<
        " [-d <geoip db>] [-h] [-k]\n\n";
}

int main(int argc, char* argv[])
{
    GeoIP* gi = NULL;
    string ip;
    map<string,unsigned> country_map;
    map<string,unsigned> city_map;
    map<string,unsigned> continent_map;
    string infile = "";
    string outfile = "";
    string filter = "";
    string geofile = GEOIPFILE;
    int c;
    bool kml = false;
    set<GeoPair> geo_set;

    opterr = 0;

    if (argc == 1) {
        usage(argv[0]);
        return 1;
    }

    while ((c = getopt (argc, argv, "i:o:f:d:hk")) != -1) {
        switch (c) {
            case 'i':
                infile = optarg;
                break;
            case 'o':
                outfile = optarg;
                break;
            case 'f':
                filter = optarg;
                break;
            case 'd':
                geofile = optarg;
                break;
            case 'h':
                usage(argv[0]);
                return 1;
                break;
            case 'k':
                kml = true;
                break;
            default:
                cerr << "Error: -" << c << " is not an option.\n\n";
        }
    }

    if (infile == "") {
        cerr << "\nInput file must be specified\n\n";
        return 1;
    }

    if (outfile == "") {
        outfile = infile + (kml == true ? ".kml" : ".csv");
        cout << "Outfile has not been specified. Using " << outfile << endl;
    }

    gi = GeoIP_open(geofile.data(), GEOIP_STANDARD | GEOIP_CHECK_CACHE);
    if (gi == NULL) {
        cerr << "Can't open GeoIP db: " << geofile << endl;
        return 2;
    }

    ifstream ifs(infile.data());
    if (!ifs.good()) {
        cerr << "Can't open input file: " << infile << endl;
        return 2;
    }
    cout << "Reading data from " << infile << endl;

    ofstream ofs(outfile.data());
    if (!ofs.good()) {
        cerr << "Can't open output file: " << outfile << endl;
        return 2;
    }
    ofs << setiosflags(ios::fixed) << setprecision(20);
    cout << "Dumping data to " << outfile << endl;

    cout << "Using filter: " << filter << endl;

    totlines = std::count(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>(), '\n');
    ifs.clear();
    ifs.seekg(0, ios::beg);
    cout << "Processing " << totlines << " lines\n";

    // write the header
    if (kml) {
        ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ofs << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
        ofs << "<Document>\n";
        ofs << "<open>0</open>\n";
        ofs << "<name>IP addresses list (" << filter << ")</name>\n";
        ofs << "<description>Locations of IP addresses list based on GeoIP database</description>\n";
        ofs << "<Style id=\"my_small_circle\">\n";
        ofs << "<IconStyle>\n";
        ofs << "<scale>0.5</scale>\n";
        ofs << "<Icon>\n";
        ofs << "<href>http://maps.google.com/mapfiles/kml/paddle/ylw-stars.png</href>\n";
        ofs << "</Icon>\n";
        ofs << "</IconStyle>\n";
        ofs << "<ListStyle>\n";
        ofs << "</ListStyle>\n";
        ofs << "</Style>\n";
    } else {
        ofs << "IP,Latitude,Longitude\n";
    }

    GeoIPRecord* record;
    GeoPair coord;

    cout << "0%";
    cout.flush();

    signal(SIGALRM, print_stats);
    alarm(1);

    while (!(ifs >> ip).eof()) {
        partial++;
        record = GeoIP_record_by_addr(gi, ip.data());

        if (!record)
            continue;

        if (!record->country_name)
            continue;

        // Filter acts on continent or country
        if (filter != "" && string(record->continent_code).find(filter) == string::npos &&
                string(record->country_name).find(filter) == string::npos)
            continue;

        coord = make_pair(record->longitude, record->latitude);

        // If we have found it before, jump to next
        if (geo_set.find(coord) != geo_set.end())
            continue;

        geo_set.insert(coord);

        if (kml) {
            ofs << "<Placemark>";
            ofs << "<name>" << ip << "</name>";
            ofs << "<styleUrl>#my_small_circle</styleUrl>\n";
            ofs << "<Icon><href>http://maps.google.com/mapfiles/kml/shapes/placemark_circle.png";
            ofs << "</href></Icon>";
            ofs << "<Point><coordinates>" << record->longitude << ",";
            ofs << record->latitude << ",0</coordinates>";
            ofs << "</Point></Placemark>\n";
        } else {
            ofs << ip << "," << record->latitude << "," << record->longitude << "\n";
        }
    }

    if (geo_set.size() == 0) {
        cerr << "No data found... Is " << filter << " the filter you wanted?\n";
    }

    ifs.close();
    GeoIP_delete(gi);

    // write the footer
    if (kml) {
        ofs << "</Document>\n</kml>\n";
    } else {
        // do nothing
    }

    ofs.close();

    cout << "\n";

    return 0;
}
