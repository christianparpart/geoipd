CXXFLAGS = -g -O3 -march=native -mtune=native -pipe
CXX = clang++

all: geoipd geoipcli

geoipd: geoipd.cpp
	${CXX} -o geoipd geoipd.cpp -std=c++11 ${CXXFLAGS}

geoipcli: geoipcli.cpp
	${CXX} -o geoipcli geoipcli.cpp -std=c++11 ${CXXFLAGS}

clean:
	rm -f geoipd geoipcli

.PHONY: clean all
