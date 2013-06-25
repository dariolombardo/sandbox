
#include <GeoIP.h>
#include <GeoIPCity.h>

#include <cassert>
#include <map>
#include <iostream>
#include <fstream>
#include <set>
#include <unistd.h>

using namespace std;

typedef pair<float,float> GeoPair;

set<GeoPair> geo_set;

#define GEOIPFILE "/usr/share/GeoIP/GeoIPCity.dat"

void usage(char* argv0)
{
    cout << "\nUsage: " << argv0 << " -i <infile> -o <outfile> -f <filter>" <<
        " -d <geoip db> -h\n\n";
}

bool checkGeoIPFile(string name)
{
    cout << "Checking GeoIP DB: " << name << endl;
    struct stat st;
    if (stat(name.c_str(), &st) == 0)
        return true;
    return false;
}

int main(int argc, char* argv[])
{
    GeoIP* gi;
    string ip;
    map<string,unsigned> country_map;
    map<string,unsigned> city_map;
    map<string,unsigned> continent_map;
    string infile = "";
    string outfile = "";
    string filter = "";
    string geofile = GEOIPFILE;
    int c;

    opterr = 0;

    if (argc == 1) {
        usage(argv[0]);
        return 1;
    }

    while ((c = getopt (argc, argv, "i:o:f:d:h")) != -1) {
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
                break;
                return 1;
            default:
                cerr << "Error: -" << c << " is not an option.\n\n";
        }
    }

    if (infile == "") {
        cerr << "\nInput file must be specified\n\n";
        return 1;
    }

    if (outfile == "") {
        outfile = infile + ".kml";
        cout << "Outfile has not been specified. Using " << outfile << endl;
    }

    gi = GeoIP_open(geofile.data(), GEOIP_STANDARD | GEOIP_CHECK_CACHE);
    assert(gi != NULL);

    cout << "Reading data from " << infile << endl;
    ifstream ifs(infile.data());
    assert(ifs.good());

    cout << "Dumping data to " << outfile << endl;
    ofstream ofs(outfile.data());
    assert(ofs.good());

    cout << "Using filter: " << filter << endl;

    // write the kml header
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

    GeoIPRecord* record;
    GeoPair coord;
    while (!(ifs >> ip).eof()) {
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

        ofs << "<Placemark>";
        ofs << "<name>" << ip << "</name>";
        ofs << "<styleUrl>#my_small_circle</styleUrl>\n";
        ofs << "<Icon><href>http://maps.google.com/mapfiles/kml/shapes/placemark_circle.png";
        ofs << "</href></Icon>";
        ofs << "<Point><coordinates>" << record->longitude << ",";
        ofs << record->latitude << ",0</coordinates>";
        ofs << "</Point></Placemark>\n";
    }

    if (geo_set.size() == 0) {
        cerr << "No data found... Is " << filter << " the filter you wanted?\n";
    }

    ifs.close();
    GeoIP_delete(gi);

    // write the footer
    ofs << "</Document>\n</kml>\n";

    ofs.close();

    return 0;
}
