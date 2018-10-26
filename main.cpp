#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <stdio.h>
#include <ios>
#include <sys/stat.h>

#define DEBUG

using namespace std;
namespace fs = std::filesystem; //this is c++17 at work, but boost_filesystem is essentially same

string IntToString(int Int){
    //this function is written to purely translate int to string without weak conversions (C-like itoa)
    std::string ResString;
    std::stringstream temp_str;
    temp_str<<Int;
    ResString = temp_str.str();
    return ResString;
}

void MakeChunk(int* ChunkNum, int* ChunkSize, string* inputfile, string* outputfile) {
    //this function grabs a numbered chunk from inputfile and creates it at the destination
    #ifdef DEBUG
    cout << "MakeChunk " << *ChunkNum << endl;
    #endif
    char *buffer = new char[*ChunkSize];
    ofstream ChunkFile;
    ifstream infile(inputfile->c_str());
    string Chunk=*outputfile;
    Chunk.append(".");
    Chunk.append(IntToString(*ChunkNum));
    ChunkFile.open(Chunk.c_str(),ios::out | ios::trunc | ios::binary);
    if (ChunkFile.is_open()) {
        infile.seekg(*ChunkNum**ChunkSize);
        infile.read(buffer, *ChunkSize);
        ChunkFile.write(buffer,infile.gcount());
        ChunkFile.close();
        #ifdef DEBUG
        cout << "Chunk " << Chunk << " generated" << endl;
        #endif
    }
    delete[] buffer;
}

void GlueChunks (string* ChunkName, string* OutputFile, int* ChunkSize, int* thrCount) {
    //this function glues the resulting chunks to one output file
    string fileName;
    ofstream outputfile;
    outputfile.open(*OutputFile, ios::out | ios::binary);

    if (outputfile.is_open()) {
        #ifdef DEBUG
        cout << *OutputFile << " opened to write" << endl;
        #endif

        for (int i=0;i<=*thrCount;i++)
        {
            // Build the filename
            fileName.clear();
            fileName.append(*ChunkName);
            fileName.append(".");
            fileName.append(IntToString(i));

            // Open chunk to read
            ifstream fileInput;
            fileInput.open(fileName.c_str(), ios::in | ios::binary);
            #ifdef DEBUG
            cout << fileName << " opened to read" << endl;
            #endif

            // If chunk opened successfully, read it and write it to the output file.
            if (fileInput.is_open()) {
                //we can work around without chunk size (by checking file size) but it's easier to give it to the function instead
                char *inputBuffer = new char[*ChunkSize];
                fileInput.read(inputBuffer,*ChunkSize);
                outputfile.write(inputBuffer,*ChunkSize);
                delete[](inputBuffer);
                fileInput.close();
            }
        }
        // Close output file.
        outputfile.close();
        #ifdef DEBUG
        cout << "File assembly complete!" << endl;
        #endif
    }
    else { cout << "Error: Unable to open file for output." << endl; }
}

int main( int argc , char *argv[ ] ) {
    //initialize empty paths
    std::string inputfile = "", outputfile = "";
    //mutex for thread-locking
    std::mutex mtx;
    //initial thread count and chunk size
    int thrCount=5;int ChunkSize=0;
    //populate them from command-line
    if (argc > 2)
    {
        inputfile = argv[1];
        outputfile = argv[2];
    }
    //Nothing to do if we're not given an output file
    if (outputfile=="") {cout << "Only input file specified" << endl; return 1;}
    if (argc > 3) {thrCount=atoi(argv[3]);};
    //TBA: fourth argument for archival, encryption, reversal
    std::vector<std::thread> threadsPool(thrCount);
    fs::path inputpath = fs::u8path(inputfile);
    fs::path outputpath = fs::u8path(outputfile);
    #ifdef DEBUG
    cout << inputfile << endl;
    cout << outputfile << endl;
    #endif
    //this is needed since we can accidentally end up with 0 data in chunks
    if (thrCount>(fs::file_size(inputpath)/2)) {cout << "Chunks too small to contain any data" << endl;; return 1;}; 
    if ((fs::file_size(inputpath)%thrCount)==0) 
    {
        ChunkSize = (fs::file_size(inputpath)/thrCount); 
    } 
    else 
    {
        // If file size does not evenly divide in our chunk/thread count, we add one more byte to the chunk size 
        // this is taken from consideration where file size divided and multiplied by the thread count always bigger than the file
        // so if we just extend the chunk a little it would not hurt
        ChunkSize = (fs::file_size(inputpath)/thrCount)+1; 
    }
    //the section above might produce a bogus and empty chunk, but that's the price I'm willing to pay to be sure all the data was taken with
    #ifdef DEBUG
    cout << "Size " << ChunkSize << endl;
    #endif
    for (int i=0;i<=thrCount;i++) {
        //critical section - mutex to avoid race condition and multiple creation of the same chunks
        mtx.lock();
        #ifdef DEBUG
        cout << "Thread " << i << endl;
        #endif
        threadsPool[i] = thread(&MakeChunk,&i,&ChunkSize,&inputfile,&outputfile);
        threadsPool[i].join();
        mtx.unlock();
    }
    //we need the exitfile since we have chunks to merge to it
    ofstream offile(outputfile);
    GlueChunks(&outputfile,&outputfile,&ChunkSize,&thrCount);
    //finish up with file system (c++17/boost_file system)
    fs::permissions(outputpath, fs::status(inputpath).permissions(), fs::perm_options::replace);
    fs::last_write_time(outputpath,fs::last_write_time(inputpath));
    #ifdef DELETE
    fs::remove(inputpath);
    #endif
    for (int i=0;i<=thrCount;i++)
    {
        //clean up
        std::string ChunkName;
        ChunkName.clear();
        ChunkName.append(outputfile);
        ChunkName.append(".");
        ChunkName.append(IntToString(i));
        #ifdef DEBUG
        cout << "to del " << ChunkName << endl;
        #endif
        fs::path chunkpath=fs::u8path(ChunkName);
        fs::remove(chunkpath);
    }
    return 0;
}
