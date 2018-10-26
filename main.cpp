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

using namespace std;
namespace fs = std::filesystem; //this is c++17 at work, but boost_filesystem is essentially same

void MakeChunk(int* ChunkNum, int* ChunkSize, string* inputfile, string* outputfile) {
    #ifdef DEBUG
    cout << "MakeChunk " << *ChunkNum << endl;
    #endif
    char *buffer = new char[*ChunkSize];
    ofstream ChunkFile;
    ifstream infile(inputfile->c_str());
    string Chunk=*outputfile;
        Chunk.append(".");
        std::stringstream temp_str;
        temp_str<<*ChunkNum;
        std::string str = temp_str.str();
        Chunk.append(str);
    ChunkFile.open(Chunk.c_str(),ios::out | ios::trunc | ios::binary);
    if (ChunkFile.is_open()) {
            infile.seekg(*ChunkNum**ChunkSize);
            infile.read(buffer, *ChunkSize);
            ChunkFile.write(buffer,infile.gcount());
            ChunkFile.close();
    }
    delete[] buffer;
}

void GlueChunks (string* ChunkName, string* OutputFile, int* ChunkSize) {
    string fileName;
    ofstream outputfile;
    outputfile.open(*OutputFile, ios::out | ios::binary);

    if (outputfile.is_open()) {
        #ifdef DEBUG
        cout << *OutputFile << " opened to write" << endl;
        #endif
        bool filefound = true;
		int i = 0;
        string x;

		while (filefound) {
			filefound = false;

			// Build the filename
			fileName.clear();
			fileName.append(*ChunkName);
			fileName.append(".");

                std::stringstream temp_str;
                temp_str<<i;
                std::string str = temp_str.str();
                fileName.append(str);

			// Open chunk to read
			ifstream fileInput;
			fileInput.open(fileName.c_str(), ios::in | ios::binary);
            #ifdef DEBUG
            cout << fileName << " opened to read" << endl;
            #endif

			// If chunk opened successfully, read it and write it to 
			// output file.
			if (fileInput.is_open()) {
				filefound = true;
				char *inputBuffer = new char[*ChunkSize];

				fileInput.read(inputBuffer,*ChunkSize);
				outputfile.write(inputBuffer,*ChunkSize);
				delete(inputBuffer);

				fileInput.close();
			}
			i++;
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
    //std::atomic<int> i;
    std::mutex mtx;
    int thrCount=1;int ChunkSize=0;
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
    //if file size does not evenly divide in our chunk/thread count, we need to add another chunk/thread
    if ((fs::file_size(inputpath)%thrCount)==0) 
    {ChunkSize = (fs::file_size(inputpath)/thrCount);} 
    else 
    {thrCount++; ChunkSize = (fs::file_size(inputpath)/thrCount);}
    for (int i=0;i<thrCount;i++) {
        //critical section - mutex to avoid race condition and multiple chunks of the same data
        mtx.lock();
        #ifdef DEBUG
        cout << "Thread " << i << endl;
        #endif
        threadsPool[i] = thread(&MakeChunk,&i,&ChunkSize,&inputfile,&outputfile);
        threadsPool[i].join();
        mtx.unlock();
    }
    //we might need that since we have chunks to merge to it
    ofstream offile(outputfile);
    GlueChunks(&outputfile,&outputfile,&ChunkSize);
    //finish up with file system (c++17/boost_file system)
    //below this line are the last steps we should take
    fs::permissions(outputpath, fs::status(inputpath).permissions(), fs::perm_options::replace);
    fs::last_write_time(outputpath,fs::last_write_time(inputpath));
    //fs::remove(inputpath); will be once we're sure
    return 0;
}
