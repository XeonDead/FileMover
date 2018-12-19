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

int main( int argc , char *argv[ ] ) {
  //initial thread count and operation mode
  unsigned long threads=5; long operationMode=0;
  std::string inPath; std::string ofPath;
  if (argc<3) {
  std::cout << "Usage: filemover $inputFile $outputFile $threads $operationMode" << std::endl;
  std::cout << "operationMode: 1=compress; 2=encrypt; 3=compress+encrypt; 4=reverse" << std::endl;
  return 0;
  }
  if (argc==3) {
    ofPath=(argv[2]);
    inPath=(argv[1]);
  }
  if (argc==4) {
    threads=strtoul(argv[3],nullptr,0);
    ofPath=(argv[2]);
    inPath=(argv[1]);
  }
  if (argc==5) {
    operationMode=strtol(argv[4],nullptr,0);
    threads=strtoul(argv[3],nullptr,0);
    ofPath=(argv[2]);
    inPath=(argv[1]);
  }

  try {
    //start sanity checks
    if (!boost::filesystem::exists(boost::filesystem::path(inPath))){
      throw std::runtime_error("Input file does not exist\n"); 
    }
    if (threads > ( boost::filesystem::file_size( boost::filesystem::path(inPath) ) / 2 )) {
      throw std::runtime_error("Chunks too small to contain any data\n"); 
    };
    if(!boost::filesystem::exists(boost::filesystem::path(ofPath).parent_path())) {
      throw std::runtime_error("Output folder does not exist\n");
    };
    if (boost::filesystem::exists(boost::filesystem::path(ofPath))) {
      std::cout << "Output file exists. Overwriting..." <<std::endl;
      boost::filesystem::remove(boost::filesystem::path(ofPath));
    };
  }
  catch (std::exception& e) {
    std::cout << e.what(); exit(1);
  };

  File inFile(inPath,operationMode,threads);
  File ofFile(ofPath,operationMode,threads);

  inFile.startMoving(threads,&ofFile);
  ofFile.glueChunks(threads);

  //finish up with file system (c++17 filesystem/boost_filesystem)
  ofFile.setPermissions(inFile.getPermissions());
  ofFile.setWriteTime(inFile.getWriteTime());
  
  #ifndef DEBUG
  boost::filesystem::remove(inFile.getFilesystemPath());
  #endif
  return 0;
}
