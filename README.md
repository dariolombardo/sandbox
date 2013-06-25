ip2kml
======

Convert a IP addresses file into a google earth KML file.

Compilation
-----------
Compile the source code with

    make

Usage
-----

Running without parameters, it gives you the help


    Usage: ./ip2kml -i <infile> -o <outfile> -f <filter> -d <geoip db> -h


Where the options are:

  * -i: the input file (mandatory)
  * -o: output file (optional: if not specified, input.kml is used)
  * -f: a filter to apply to continent code or country name (optional)
  * -d: the location of GeoIP database (optional)
  * -h: the help
