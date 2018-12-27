#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>
#include <cstring>
#include <boost/filesystem.hpp>
#include <future>
#include <mutex>
#include <algorithm>
#include <vector>
#include <iterator>
#include <exception>

#ifndef CB_FILE_H
#include "file.h"
#endif

//#define DEBUG

void File::setPath(const std::string path) 
{
  path_=path;
  fspath_=boost::filesystem::path(path);
};

std::string File::getPath() const 
{
  return path_;
};

boost::filesystem::path File::getFilesystemPath() const 
{
  return fspath_;
};

void File::setPermissions(const boost::filesystem::perms permissions)
{
  boost::filesystem::permissions(fspath_, permissions);
};

boost::filesystem::perms File::getPermissions() const 
{
  return boost::filesystem::status(path_).permissions();
};

void File::setWriteTime(const std::time_t writeTime)
{
  boost::filesystem::last_write_time(fspath_, writeTime);
};

std::time_t File::getWriteTime() const 
{
  return boost::filesystem::last_write_time(path_);
};

unsigned long File::getChunkSize() const
{
  return chunkSize_;
};

unsigned long File::getFileSize() const
{
  return fileSize_;
};

unsigned long File::getTransferSize() const
{
  return transferSize_;
};

File::File(std::string path, int initParameters, unsigned long threads) {
  setPath(std::move(path));
  if(boost::filesystem::exists(getFilesystemPath())) {
    fileSize_ = boost::filesystem::file_size(getFilesystemPath());
    if ((fileSize_ % threads) == 0) {
      chunkSize_ = (fileSize_ / threads);
    } else {
      chunkSize_ = (fileSize_ / threads)+1;
    };
  }
  if (fileSize_ > 1048576) { transferSize_ = 1048576;}  //2^20, 1mb chunk
    else if (fileSize_ > 1024) { transferSize_ = 1024;} //2^10 1kb chunk
      else transferSize_ = chunkSize_; //small enough to fit in memory
  #ifdef DEBUG
    //std::cout << "Transfer Size is:" << transferSize_ << std::endl;
  #endif
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

int File::startMoving(const unsigned long threads, const File* outputFile){
  if (parameters_.toInverse) {
    /*workaround in code*/
  }
  if (parameters_.toEncrypt && parameters_.toCompress) {
    /*use both 1 and 2*/
  }
  if (parameters_.toEncrypt) {
    /*use openssl with some encryption key to pre-process and encrypt the file*/
  }
  if (parameters_.toCompress) {
    /*boost::iostream::zlib to compress file on the fly*/
  }

  std::vector<std::thread> threadPool;
  for (ulong i=0;i<threads;i++) {
    threadPool.emplace_back(std::thread(&File::makeChunk,this,i,outputFile));
  };
  for (ulong i=0;i<threads;i++) {
    threadPool[i].join();
  };

  return 0;
};

int File::makeChunk(const File* inputFile, const int chunkNum, const File* outputFile){
  //this function grabs a numbered chunk from inputFile and creates it at the destination
  #ifdef DEBUG
  std::cout << "start work on chunk number " << chunkNum << " thread " << std::this_thread::get_id() << std::endl;
  #endif
  int fullChunkSize=inputFile->getChunkSize();
  int transferSize=inputFile->getTransferSize();
  std::ofstream chunkFile;
  std::ifstream inFile(inputFile->path_);
  std::string chunk=outputFile->path_;
  chunk.append(".");
  chunk.append(std::to_string(chunkNum));
  chunkFile.open(chunk.c_str(),std::ios::out | std::ios::trunc | std::ios::binary);
  if (chunkFile.is_open()) {
    inFile.seekg(chunkNum*fullChunkSize);
    for (int i=0;i<fullChunkSize;i+=transferSize){
      auto *buffer = new char[transferSize];
      inFile.read(buffer, transferSize);
        if(inputFile->parameters_.toInverse) {
          std::reverse(buffer,buffer+strlen(buffer));
        }//no longer loses data with strlen
      #ifdef DEBUG
	//std::cout << "Put " << i << " crabs in bucket" << std::endl;
      #endif
      chunkFile.write(buffer,transferSize);
      delete[] buffer;
    }
    chunkFile.close();
    #ifdef DEBUG
    std::cout << "chunk " << chunk << " generated"<< std::endl;
    #endif
  }
  return 0;
};

int File::glueChunks(const unsigned long threads) {
  //this function glues the resulting chunks to one output file
  #ifdef DEBUG
  std::cout << "Starting glueChunks" << std::endl;
  #endif
  std::ofstream outputFile;
  outputFile.open(path_, std::ios::out | std::ios::binary);

  if (outputFile.is_open()) {
    #ifdef DEBUG
    std::cout << path_ << " opened to write" << std::endl;
    #endif

    for (ulong i=0;i<threads;i++) {
      // Build the filename
      std::string chunkFileName;
      chunkFileName.clear();
      chunkFileName.append(path_);
      chunkFileName.append(".");
      if (parameters_.toInverse) {
        ulong j = threads-i-1;
        chunkFileName.append(std::to_string(j));
      } else chunkFileName.append(std::to_string(i));

      // Open chunk to read
      std::ifstream chunkInputFile;
      chunkInputFile.open(chunkFileName.c_str(), std::ios::in | std::ios::binary);
      #ifdef DEBUG
      std::cout << chunkFileName << " opened to read" << std::endl;
      #endif

      // If chunk opened successfully, read it and write it to the output file.
      if (chunkInputFile.is_open() && boost::filesystem::file_size(boost::filesystem::path(chunkFileName))!=0) {
        unsigned long thisChunkSize = boost::filesystem::file_size(boost::filesystem::path(chunkFileName));
        auto *buffer = new char[thisChunkSize];
        // After a little brainstorm, using actual chunk size is a better idea
        chunkInputFile.read(buffer,thisChunkSize);
        outputFile.write(buffer,thisChunkSize);
        delete[](buffer);
        chunkInputFile.close();
      }
    #ifdef DEBUG
    std::cout << "to del " << chunkFileName << std::endl;
    #endif
    boost::filesystem::path chunkPath=boost::filesystem::path(chunkFileName);
    boost::filesystem::remove(chunkPath);
  }
  // Close output file.
  outputFile.close();
  #ifdef DEBUG
  std::cout << "File assembly complete!" << std::endl;
  #endif
  } else {
    std::cout << "Error: Unable to open file for output." << std::endl; exit(1);
  }
  return 0;
};
 
