#include <iostream>
#include <fstream>
#include <ios>
#include <filesystem>
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>
#include <list>
#include <iterator>

//#define DEBUG

using namespace std;
namespace fs = std::filesystem; //this is c++17 at work, but boost_filesystem is essentially same

class File {
    public:
        string Path;
        fs::path FsPath;
        File(){};
        File(string Path);
};

File::File(string Path) {
    FsPath = fs::u8path(Path);
};

class Chunk {
    public:
        vector<char> ChunkData;
        Chunk(){};
};

class InputFile: public File {
    public:
        unsigned int Parameters:3;
        fs::perms FilePermissions;
        vector<Chunk> Chunks;
        InputFile(){};
        InputFile(string Path);
        InputFile(string Path, unsigned int Parameters);
        int StartMoving(InputFile InputFile, const int* Threads, const int* ChunkSize, vector<Chunk> Chunks);
        int StartInvMoving(InputFile InputFile, const int* Threads, const int* ChunkSize, vector<Chunk> Chunks);
        int MakeChunks(InputFile InputFile, const int *ChunkNum, const int *ChunkSize, vector<Chunk> Chunks);
        int InvMakeChunks(InputFile InputFile, const int *ChunkNum, const int *ChunkSize, vector<Chunk> Chunks);
};

InputFile::InputFile(string Path) {
    FilePermissions = fs::status(Path).permissions();
    Parameters = 0;
};

InputFile::InputFile(string Path,unsigned int InitParameters) {
    FilePermissions = fs::status(Path).permissions();
    Parameters = InitParameters;
};

int InputFile::StartMoving(InputFile InputFile, const int* Threads, const int* ChunkSize, vector<Chunk> Chunks){
    if (InputFile.Parameters==4) {InputFile.StartInvMoving(InputFile, Threads, ChunkSize, Chunks); return 0;}
    if (InputFile.Parameters==3) {/*use both 1 and 2*/}
    if (InputFile.Parameters==2) {/*use openssl with some encryption key to preprocess and encrypt the file*/}
    if (InputFile.Parameters==1) {/*boost::iostream::zlib to compress file on the fly*/}
    
    ifstream OpenedFile(InputFile.Path.c_str(), ios::binary);

    for (int i=1;i<*Threads;i++) {
    }

    return 0;
};

int InputFile::StartInvMoving(InputFile InputFile, const int* Threads, const int* ChunkSize, vector<Chunk> Chunks){    
    ifstream OpenedFile(InputFile.Path.c_str(), ios::binary);

    for (int i=1;i<*Threads;i++) {
    }

    reverse(Chunks.begin(),Chunks.end());
    return 0;
};

int InputFile::MakeChunks(InputFile InputFile, const int *ChunkNum, const int *ChunkSize, vector<Chunk> Chunks){
    Chunk Chunk;
    copy(((*ChunkNum-1)**ChunkSize),*ChunkNum**ChunkSize, Chunk.ChunkData);
    Chunks[*ChunkNum]=Chunk;
    return 0;
};

int InputFile::InvMakeChunks(InputFile InputFile, const int *ChunkNum, const int *ChunkSize, vector<Chunk> Chunks){
    Chunk Chunk;
    reverse_copy(((*ChunkNum-1)**ChunkSize),*ChunkNum**ChunkSize, Chunk.ChunkData);
    Chunks[*ChunkNum]=Chunk;
    return 0;
};

class OutputFile: public File {
    public:
        OutputFile(){};
        OutputFile(string Path);
        int GlueChunks(InputFile);
};

int OutputFile::GlueChunks(InputFile) {
    /*refactored GlueChunks from below*/
    return 0;
};

