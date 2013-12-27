ip2geo
======

Convert a IP addresses file into a CSV file or a google earth KML file.

Compilation
-----------
Compile the source code with

    make

Usage
-----

Running without parameters, it gives you the help


    Usage: ./ip2kml -i <infile> [-o <outfile>] [-f <filter>] [-d <geoip db>] [-h] [-k]


Where the options are:

  * -i: the input file (mandatory)
  * -o: output file (optional: if not specified, <input>.csv/.kml is used)
  * -f: a filter to apply to continent code or country name (optional)
  * -d: the location of GeoIP database (optional, default file used otherwise)
  * -h: the help
  * -k: output file in KML format (optional, csv otherwise)
