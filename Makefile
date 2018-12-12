CXX=g++
CXXFLAGS=-Wall -std=c++11 -lboost_system -lboost_filesystem -pthread

all: filemover

SOURCES = file.cpp main.cpp

filemover: main.cpp file.cpp
	$(CXX) $(CXXFLAGS) -o filemover $(SOURCES)

clean: 
	rm -f filemover
