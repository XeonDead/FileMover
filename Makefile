CXX=g++
CXXFLAGS=-Wall -std=c++11 -lboost_system -lboost_filesystem -pthread

all: filemover

filemover: main.cpp
	$(CXX) main.cpp $(CXXFLAGS) -o filemover

clean: 
	rm -f filemover
