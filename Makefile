CXX=g++
CXXFLAGS=-Wall -std=c++17 -lstdc++fs

all: filemover

filemover: main.cpp
	$(CXX) main.cpp $(CXXFLAGS) -o filemover

clean: 
	rm -f filemover
