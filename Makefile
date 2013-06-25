CFLAGS=-Wall -Werror
LDFLAGS=-lGeoIP
CXX?=g++

all:
	$(CXX) $(CFLAGS) ip2kml.cpp -o ip2kml $(LDFLAGS)

clean:
	rm -rf ip2kml
