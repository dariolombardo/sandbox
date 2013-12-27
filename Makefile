CFLAGS+=-Wall -Werror -g
LDFLAGS+=-lGeoIP
CXX?=g++

all:
	$(CXX) $(CFLAGS) ip2geo.cpp -o ip2geo $(LDFLAGS)

clean:
	rm -rf ip2geo
