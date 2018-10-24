#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>

using namespace std;
namespace fs = std::filesystem; //this is c++17 at work, but boost_filesystem is essentially same

int main( int argc , char *argv[ ] ) {
    //initialize empty paths
    std::string inputfile = "",outputfile = "";
    //populate them from command-line
    if (argc > 2)
    {
        inputfile = argv[1];
        outputfile = argv[2];
    } 
    //TBA: third argument for archival, encryption, reversal
    #ifdef DEBUG
    cout << inputfile << endl;
    cout << outputfile << endl;
    #endif
    //Nothing to do if we're not given an output file
    if (outputfile=="") {cout << "Only input file specified" << endl; return 1;}
    //start up with filesystem (c++17/boost_filesystem)
    fs::path inputpath = fs::u8path(inputfile);
    fs::path outputpath = fs::u8path(outputfile);
    ofstream file(outputfile); //and no other way
    fs::resize_file(outputpath, 1024);
    fs::permissions(outputpath, fs::status(inputpath).permissions(), fs::perm_options::replace);
    fs::last_write_time(outputpath,fs::last_write_time(inputpath));
    //fs::remove(inputpath); will be once we're sure
    return 0;
}