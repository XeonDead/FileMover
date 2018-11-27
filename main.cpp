#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <future>
#include <mutex>
#include <algorithm>
#include <vector>
#include <iterator>

#ifndef CB_FILE_H
#include "file.h"
#endif

//#define DEBUG

using namespace std;
namespace fs = std::filesystem; //this is c++17 at work, but boost_filesystem is essentially same

void File::setPath(const std::string path) 
{
  path_=path;
  fspath_=std::filesystem::u8path(path);
};

string File::getPath() const 
{
  return path_;
};

fs::path File::getFilesystemPath() const 
{
  return fspath_;
};

void File::setPermissions(const fs::perms permissions)
{
  fs::permissions(fspath_, permissions, fs::perm_options::replace);
};

fs::perms File::getPermissions() const 
{
  return std::filesystem::status(path_).permissions();
};

void File::setWriteTime(const fs::file_time_type writeTime)
{
  fs::last_write_time(fspath_, writeTime);
};

fs::file_time_type File::getWriteTime() const 
{
  return std::filesystem::last_write_time(path_);
};

int File::getChunkSize() const 
{
  return chunkSize_;
};

int File::getFileSize() const 
{
  return fileSize_;
};

File::File(string path, int initParameters) {
  setPath(path_);
  if (fs::exists(getFilesystemPath())) {
  cout << "Output file exists. Overwriting..." <<endl;
  fs::remove(getFilesystemPath());
  };
  if (initParameters == 4) {
    parameters_.toInverse=true;
  };
  if (initParameters == 3) {
    parameters_.toEncrypt=true;
    parameters_.toCompress=true;
  };
  if (initParameters == 2) {
    parameters_.toEncrypt=true;
  };
  if (initParameters == 1) {
    parameters_.toCompress=true;
  };
};

File::File(string path, int initParameters, int threads) {
  setPath(path);
  fileSize_ = fs::file_size(getFilesystemPath());
  if ((fileSize_ % threads) == 0) {
    chunkSize_ = (fileSize_ / threads);
  } else {
    chunkSize_ = (fileSize_ / threads)+1;
  };
  if (initParameters == 4) {
    parameters_.toInverse=true;
  };
  if (initParameters == 3) {
    parameters_.toEncrypt=true;
    parameters_.toCompress=true;
  };
  if (initParameters == 2) {
    parameters_.toEncrypt=true;
  };
  if (initParameters == 1) {
    parameters_.toCompress=true;
  };
};

int File::startMoving(const int threads, const File* outputFile){
  if (parameters_.toInverse) {
    /*workarounded in code*/
  }
  if (parameters_.toEncrypt && parameters_.toCompress) {
    /*use both 1 and 2*/
  }
  if (parameters_.toEncrypt) {
    /*use openssl with some encryption key to preprocess and encrypt the file*/
  }
  if (parameters_.toCompress) {
    /*boost::iostream::zlib to compress file on the fly*/
  }

  vector<future<int>> threadPool(threads);
  for (int i=0;i<threads;i++) {
    threadPool.push_back(async(launch::async,&File::makeChunk,this,i,outputFile));
  };

  return 0;
};

int File::makeChunk(const File* inputFile, const int chunkNum, const File* outputFile){
  //this function grabs a numbered chunk from inputFile and creates it at the destination
  #ifdef DEBUG
  cout << "start work on chunk number " << chunkNum << " thread " << std::this_thread::get_id() << endl;
  #endif
  char *buffer = new char[inputFile->getChunkSize()];
  ofstream chunkFile;
  ifstream inFile(inputFile->path_);
  string chunk=outputFile->path_;
  chunk.append(".");
  chunk.append(to_string(chunkNum));
  chunkFile.open(chunk.c_str(),ios::out | ios::trunc | ios::binary);
  if (chunkFile.is_open()) {
    inFile.seekg(chunkNum*(inputFile->getChunkSize()));
    inFile.read(buffer, inputFile->getChunkSize());
    if(inputFile->parameters_.toInverse) {
      reverse(buffer,buffer+strlen(buffer)); 
    }//no longer loses data with strlen
    chunkFile.write(buffer,inFile.gcount());
    chunkFile.close();
    #ifdef DEBUG
    cout << "chunk " << chunk << " generated"<< endl;
    #endif
  }
  delete[] buffer;
  return 0;
};

int File::glueChunks(const int threads) {
  //this function glues the resulting chunks to one output file
  #ifdef DEBUG
  cout << "Starting glueChunks" << endl;
  #endif
  ofstream outputFile;
  outputFile.open(path_, ios::out | ios::binary);

  if (outputFile.is_open()) {
    #ifdef DEBUG
    cout << path_ << " opened to write" << endl;
    #endif

    for (int i=0;i<threads;i++) {
      // Build the filename
      string chunkFileName;
      chunkFileName.clear();
      chunkFileName.append(path_);
      chunkFileName.append(".");
      if (parameters_.toInverse) {
        int j = threads-i-1;
        chunkFileName.append(to_string(j));
      } else chunkFileName.append(to_string(i));

      // Open chunk to read
      ifstream chunkInputFile;
      chunkInputFile.open(chunkFileName.c_str(), ios::in | ios::binary);
      #ifdef DEBUG
      cout << chunkFileName << " opened to read" << endl;
      #endif

      // If chunk opened successfully, read it and write it to the output file.
      if (chunkInputFile.is_open() && fs::file_size(fs::u8path(chunkFileName))!=0) {
        int chunkSize = fs::file_size(fs::u8path(chunkFileName));
        char *buffer = new char[chunkSize];
        // After a little brainstorm, using actual chunk size is a better idea
        chunkInputFile.read(buffer,chunkSize);
        outputFile.write(buffer,chunkSize);
        delete[](buffer);
        chunkInputFile.close();
      }
    #ifdef DEBUG
    cout << "to del " << chunkFileName << endl;
    #endif
    fs::path chunkPath=fs::u8path(chunkFileName);
    fs::remove(chunkPath);
  }
  // Close output file.
  outputFile.close();
  #ifdef DEBUG
  cout << "File assembly complete!" << endl;
  #endif
  } else {
    cout << "Error: Unable to open file for output." << endl; exit(1);
  }
  return 0;
};

int main( int argc , char *argv[ ] ) {
  //initial thread count, chunk size and operation mode
  int threads=5; int operationMode=0;
  string inPath=""; string ofPath="";
  if (argc<3) {
  cout << "Usage: filemover $inputFile $outputFile $threads $operationMode" << endl;
  cout << "operationMode: 1=compress; 2=encrypt; 3=compress+encrypt; 4=reverse" << endl;
  return 0;
  }
  if (argc==3) {
    ofPath=(argv[2]);
    inPath=(argv[1]);
  }
  if (argc==4) {
    threads=atoi(argv[3]);
    ofPath=(argv[2]);
    inPath=(argv[1]);
  }
  if (argc==5) {
    operationMode=atoi(argv[4]);
    threads=atoi(argv[3]);
    ofPath=(argv[2]);
    inPath=(argv[1]);
  }

  try {
    //start sanity checks
    if (!fs::exists(fs::u8path(inPath))){
      throw runtime_error("Input file does not exist\n"); 
    }
    if (threads > ( fs::file_size( fs::u8path(inPath) ) / 2 )) {
      throw runtime_error("Chunks too small to contain any data\n"); 
    };
    if(!fs::exists(fs::u8path(ofPath).parent_path())) {
      throw runtime_error("Output folder does not exist\n");
    };
  }
  catch (exception& e) {
    cout << e.what(); exit(1);
  };

  File inFile(inPath,operationMode,threads);
  File ofFile(ofPath,operationMode);

  inFile.startMoving(threads,&ofFile);
  ofFile.glueChunks(threads);

  //finish up with file system (c++17 filesystem/boost_filesystem)
  ofFile.setPermissions(inFile.getPermissions());
  ofFile.setWriteTime(inFile.getWriteTime());
  
  #ifndef DEBUG
  fs::remove(inFile.getFilesystemPath());
  #endif
  return 0;
}
