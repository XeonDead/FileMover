#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>
#include <iterator>

//#define DEBUG

using namespace std;
namespace fs = std::filesystem; //this is c++17 at work, but boost_filesystem is essentially same

union parameters {
    bool toCompress = false;
    bool toEncrypt;
    bool toInverse;
};

class outputFile {
    public:
        string path;
        fs::path fspath;
        parameters parameters;
        outputFile(){};
        outputFile(string path){
            this->path=path;
            fspath = fs::u8path(path);
            //it is expected for the output file to not exist before running the program
            //unless...
            if(!fs::exists(fspath.parent_path())) {throw runtime_error("Output folder does not exist"); exit(1);}
        };
        int glueChunks(const int* threads);
};

class inputFile {
    public:
        string path;
        fs::path fspath;
        parameters parameters;
        fs::perms filePermissions;
        fs::file_time_type last_write_time;
        inputFile(){};
        inputFile(string path) {
            this->path=path;
            fspath = fs::u8path(path);
            if (fs::exists(fspath)){
                filePermissions = fs::status(path).permissions();
                last_write_time = fs::last_write_time(path);
            } else { throw runtime_error("Input file does not exist"); exit(1); }
        };
        inputFile(string path, int InitParameters) {
            this->path=path;
            fspath = fs::u8path(path);
            if (fs::exists(fspath)){
                filePermissions = fs::status(path).permissions();
                last_write_time = fs::last_write_time(path);
            } else { throw runtime_error("Input file does not exist"); exit(1); }
            if (InitParameters==4) {this->parameters.toInverse=true;}
            if (InitParameters==3) {this->parameters.toEncrypt=true;this->parameters.toCompress=true;}
            if (InitParameters==2) {this->parameters.toEncrypt=true;}
            if (InitParameters==1) {this->parameters.toCompress=true;}
        };
        int startMoving(const int* threads, const int* chunkSize, outputFile* outputFile);
        static int makeChunk(const inputFile* inputFile, const int *chunkNum, const int *chunkSize, const outputFile* outputFile);
};

int outputFile::glueChunks(const int* threads) {
    //this function glues the resulting chunks to one output file
    #ifdef DEBUG
    cout << "Starting glueChunks" << endl;
    #endif
    ofstream outputFile;
    outputFile.open(this->path, ios::out | ios::binary);

    if (outputFile.is_open()) {
        #ifdef DEBUG
        cout << this->path << " opened to write" << endl;
        #endif

        for (int i=0;i<*threads;i++)
        {
            // Build the filename
            string chunkFileName;
            chunkFileName.clear();
            chunkFileName.append(this->path);
            chunkFileName.append(".");
            if (this->parameters.toInverse){
                int j = *threads-i-1;
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
        }
        // Close output file.
        outputFile.close();
        #ifdef DEBUG
        cout << "File assembly complete!" << endl;
        #endif
    }
    else { runtime_error("Error: Unable to open file for output."); exit(1); }
    return 0;
};

int inputFile::startMoving(const int* threads, const int* chunkSize, outputFile* outputFile){
    if (this->parameters.toInverse) {/*workarounded in code*/}
    if (this->parameters.toEncrypt && this->parameters.toCompress) {/*use both 1 and 2*/}
    if (this->parameters.toEncrypt) {/*use openssl with some encryption key to preprocess and encrypt the file*/}
    if (this->parameters.toCompress) {/*boost::iostream::zlib to compress file on the fly*/}

    mutex mtx;
    vector<thread> threadPool(*threads);

    for (int i=0;i<*threads;i++) {
        //critical section - mutex to avoid race condition and multiple creation of the same chunks
        mtx.lock();
        #ifdef DEBUG
        cout << "Thread " << i << endl;
        #endif
        threadPool[i] = thread(&inputFile::makeChunk,this,&i,chunkSize,outputFile);
        threadPool[i].join();
        mtx.unlock();
    }

    return 0;
};

int inputFile::makeChunk(const inputFile* inputFile, const int *chunkNum, const int *chunkSize, const outputFile* outputFile){
    //this function grabs a numbered chunk from inputFile and creates it at the destination
    #ifdef DEBUG
    cout << "makeChunk " << *chunkNum << endl;
    #endif
    char *buffer = new char[*chunkSize];
    ofstream chunkFile;
    ifstream inFile(inputFile->path);
    string chunk=outputFile->path;
    chunk.append(".");
    chunk.append(to_string(*chunkNum));
    chunkFile.open(chunk.c_str(),ios::out | ios::trunc | ios::binary);
    if (chunkFile.is_open()) {
        inFile.seekg(*chunkNum**chunkSize);
        inFile.read(buffer, *chunkSize);
        if(inputFile->parameters.toInverse) {reverse(buffer,buffer+strlen(buffer)); }//no longer loses data with strlen
        chunkFile.write(buffer,inFile.gcount());
        chunkFile.close();
        #ifdef DEBUG
        cout << "chunk " << chunk << " generated" << endl;
        #endif
    }
    delete[] buffer;
    return 0;
};

int main( int argc , char *argv[ ] ) {
        //initial thread count, chunk size and operation mode
    int threads=5; int chunkSize=0; int operationMode=0;
    string inPath=""; string ofPath="";
    switch (argc)
    {
        case 5:
        {
            operationMode=atoi(argv[4]);
            threads=atoi(argv[3]);
            ofPath=(argv[2]);
            inPath=(argv[1]);
            break;
        }
        case 4:
        {
            threads=atoi(argv[3]);
            ofPath=(argv[2]);
            inPath=(argv[1]);
            break;
        }
        case 3:
        {
            ofPath=(argv[2]);
            inPath=(argv[1]);
            break;
        }
        case 2:
        case 1:
        case 0:
        {
            cout << "Usage: filemover $inputFile $outputFile $threads $operationMode" << endl;
            cout << "operationMode: 1=compress; 2=encrypt; 3=compress+encrypt; 4=reverse" << endl;
            return 0;
        }
    }

    inputFile inFile(inPath,operationMode);
    outputFile ofFile(ofPath);
    ofFile.parameters=inFile.parameters;

    #ifdef DEBUG
    cout << "inputFile: " << inFile.path << endl;
    cout << "outputFile: " <<  ofFile.path << endl;
    #endif
        //this is needed since we can accidentally end up with 0 data in chunks
    if (threads>(fs::file_size(inFile.path)/2)) {throw runtime_error("Chunks too small to contain any data"); exit(1);};
    if ((fs::file_size(inFile.path)%threads)==0)
    {chunkSize = (fs::file_size(inFile.path)/threads); }
    else {chunkSize = (fs::file_size(inFile.path)/threads)+1; }
        // If file size does not evenly divide in our chunk/thread count, we add one more byte to the chunk size
        // this is taken from consideration where if file size has a modulus after division we can be certain that if chunk size will be bigger it will cover
        // the entire file so if we just extend the chunk a little it will contain the file entirely
    #ifdef DEBUG
    cout << "Size " << chunkSize << endl;
    #endif

    inFile.startMoving(&threads,&chunkSize,&ofFile);
    ofFile.glueChunks(&threads);

        //finish up with file system (c++17 filesystem/boost_filesystem)
    fs::permissions(ofFile.fspath,inFile.filePermissions, fs::perm_options::replace);
    fs::last_write_time(ofFile.fspath,inFile.last_write_time);
    #ifndef DEBUG
    fs::remove(inFile.fspath);
    #endif
    for (int i=0;i<threads;i++)
    {
        //clean up
        string chunkName;
        chunkName.clear();
        chunkName.append(ofFile.path);
        chunkName.append(".");
        chunkName.append(to_string(i));
        #ifdef DEBUG
        cout << "to del " << chunkName << endl;
        #endif
        fs::path chunkpath=fs::u8path(chunkName);
        fs::remove(chunkpath);
    }
    return 0;
}
