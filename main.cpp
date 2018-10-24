#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <thread>
#include <cstdlib>
#include <stdio.h>
#include <ios>
#include <sys/stat.h>

using namespace std;
namespace fs = std::filesystem; //this is c++17 at work, but boost_filesystem is essentially same

void pushChunk(int* ChunkNum, int* ChunkSize, string* inputfile, string* outputfile) {
    char *buffer = new char[*ChunkSize];
    ofstream ChunkFile;
    //ifstream inputfile;
    ifstream infile(inputfile->c_str());
    string Chunk=*outputfile;
        //Chunk.clear();
        //Chunk.append(*outputfile.c_str());
        Chunk.append(".");
        std::stringstream temp_str;
        temp_str<<*ChunkNum;
        std::string str = temp_str.str();
        Chunk.append(str);
    ChunkFile.open(Chunk.c_str(),ios::out | ios::trunc | ios::binary);
    if (ChunkFile.is_open()) {
            infile.read(buffer, *ChunkSize);
            ChunkFile.write(buffer,infile.gcount());
            ChunkFile.close();
    }
    delete[] buffer;
}

int main( int argc , char *argv[ ] ) {
    //initialize empty paths
    std::string inputfile = "",outputfile = "";
    int thrCount=1;
    //populate them from command-line
    if (argc > 2)
    {
        inputfile = argv[1];
        outputfile = argv[2];
    } 
    if (argc > 3) {thrCount=atoi(argv[3]);};
    //TBA: fourch argument for archival, encryption, reversal
    std::vector<std::thread> threadsPool(thrCount);
    fs::path inputpath = fs::u8path(inputfile);
    fs::path outputpath = fs::u8path(outputfile);
    #ifdef DEBUG
    cout << inputfile << endl;
    cout << outputfile << endl;
    #endif
    //Nothing to do if we're not given an output file
    if (outputfile=="") {cout << "Only input file specified" << endl; return 1;}
    //start up with filesystem (c++17/boost_filesystem)
    ofstream offile(outputfile);
    //now I need to figure out how to make many threads
    int ChunkSize = (fs::file_size(inputpath)/thrCount)+1;
    for (int i=0;i<thrCount;i++) {
        threadsPool[thrCount] = thread(&pushChunk,&i,&ChunkSize,&inputfile,&outputfile);
    }
    for (int i=0;i<thrCount;i++) {
        threadsPool[i].join();
    }
    //below this line are the last steps we should take
    //fs::resize_file(outputpath, 1024);
    fs::permissions(outputpath, fs::status(inputpath).permissions(), fs::perm_options::replace);
    fs::last_write_time(outputpath,fs::last_write_time(inputpath));
    //fs::remove(inputpath); will be once we're sure
    return 0;
}