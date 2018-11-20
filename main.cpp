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

//#define DEBUG

using namespace std;
namespace fs = std::filesystem; //this is c++17 at work, but boost_filesystem is essentially same

struct parameters {
    bool toCompress = false;
    bool toEncrypt = false;
    bool toInverse = false;
};

class outputFile;

class inputFile {
    protected:
        fs::path fspath;
        parameters parameters;
        int fileSize; int chunkSize;
    public:
        string path;
        inputFile(){};
        inputFile(const string *path, const int *threads);
        inputFile(const string *path, const int *InitParameters, const int *threads);
        int startMoving(const int* threads, outputFile* outputFile);
        static int makeChunk(const inputFile* inputFile, const int chunkNum, const outputFile* outputFile);
        fs::perms getPermissions() const {return fs::status(this->path).permissions();};
        fs::file_time_type getWriteTime() const {return fs::last_write_time(this->path);};
        int getChunkSize() const {return this->chunkSize;};
        fs::path getFilesystemPath() const {return this->fspath;};
};

class outputFile {
    protected:
        fs::path fspath;
        parameters parameters;
    public:
        string path;
        outputFile(){};
        outputFile(const string *path, const int *InitParameters){
            this->path=*path;
            fspath = fs::u8path(this->path);
            //it is expected for the output file to not exist before running the program
            //unless...
            if(!fs::exists(fspath.parent_path())) {throw runtime_error("Output folder does not exist");}
        if (*InitParameters == 4) {this->parameters.toInverse=true;}
        if (*InitParameters == 3) {this->parameters.toEncrypt=true;this->parameters.toCompress=true;}
        if (*InitParameters == 2) {this->parameters.toEncrypt=true;}
        if (*InitParameters == 1) {this->parameters.toCompress=true;}
        };
        int glueChunks(const int* threads);
        void setPermissions(const inputFile* inputFile){
            fs::permissions(this->fspath,inputFile->getPermissions(), fs::perm_options::replace);
        }
        void setWriteTime(const inputFile* inputFile){
            fs::last_write_time(this->fspath,inputFile->getWriteTime());
        }
};

inputFile::inputFile(const string *path, const int *threads) {
    this->path=*path;
    fspath = fs::u8path(this->path);
    if (fs::exists(fspath)){
        fileSize = fs::file_size(this->path);    
        if ( *threads > ( fileSize / 2 )) {throw runtime_error("Chunks too small to contain any data"); };
        if ((fileSize % *threads) == 0) {chunkSize = (fileSize / *threads); }
        else {chunkSize = (fileSize / *threads)+1; }
            #ifdef DEBUG
            cout << "Size " << chunkSize << endl;
            #endif                
        // If file size does not evenly divide in our chunk/thread count, we add one more byte to the chunk size
        // this is taken from consideration where if file size has a modulus after division we can be certain that if chunk size will be bigger it will cover
        // the entire file so if we just extend the chunk a little it will contain the file entirely
    } else { throw runtime_error("Input file does not exist"); }
};

inputFile::inputFile(const string *path, const int *InitParameters, const int *threads) {
    this->path=*path;
    fspath = fs::u8path(this->path);
    if (fs::exists(fspath)){
        fileSize = fs::file_size(fspath);
        if ( *threads > ( fileSize / 2 )) {throw runtime_error("Chunks too small to contain any data"); };
        if ((fileSize % *threads) == 0) {chunkSize = (fileSize / *threads);}
        else {chunkSize = (fileSize / *threads)+1;}
            #ifdef DEBUG
            cout << "Size " << chunkSize << endl;
            #endif                
    } else { throw runtime_error("Input file does not exist"); }
    if (*InitParameters == 4) {this->parameters.toInverse=true;}
    if (*InitParameters == 3) {this->parameters.toEncrypt=true;this->parameters.toCompress=true;}
    if (*InitParameters == 2) {this->parameters.toEncrypt=true;}
    if (*InitParameters == 1) {this->parameters.toCompress=true;}
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
    }
    else { throw runtime_error("Error: Unable to open file for output.");}
    return 0;
};

int inputFile::startMoving(const int* threads, outputFile* outputFile){
    if (this->parameters.toInverse) {/*workarounded in code*/}
    if (this->parameters.toEncrypt && this->parameters.toCompress) {/*use both 1 and 2*/}
    if (this->parameters.toEncrypt) {/*use openssl with some encryption key to preprocess and encrypt the file*/}
    if (this->parameters.toCompress) {/*boost::iostream::zlib to compress file on the fly*/}

    vector<future<int>> threadPool(*threads);
    for (int i=0;i<*threads;i++) {
        threadPool.push_back(async(launch::async,&inputFile::makeChunk,this,i,outputFile));
    }

    return 0;
};

int inputFile::makeChunk(const inputFile* inputFile, const int chunkNum, const outputFile* outputFile){
    //this function grabs a numbered chunk from inputFile and creates it at the destination
    #ifdef DEBUG
    cout << "start work on chunk number " << chunkNum << " thread " << std::this_thread::get_id() << endl;
    #endif
    char *buffer = new char[inputFile->getChunkSize()];
    ofstream chunkFile;
    ifstream inFile(inputFile->path);
    string chunk=outputFile->path;
    chunk.append(".");
    chunk.append(to_string(chunkNum));
    chunkFile.open(chunk.c_str(),ios::out | ios::trunc | ios::binary);
    if (chunkFile.is_open()) {
        inFile.seekg(chunkNum*(inputFile->getChunkSize()));
        inFile.read(buffer, inputFile->getChunkSize());
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
    int threads=5; int operationMode=0;
    string inPath=""; string ofPath="";
    if (argc==5) {
        operationMode=atoi(argv[4]);
        threads=atoi(argv[3]);
        ofPath=(argv[2]);
        inPath=(argv[1]);
    }
    if (argc==4) {
        threads=atoi(argv[3]);
        ofPath=(argv[2]);
        inPath=(argv[1]);
    }
    if (argc==3) {
        ofPath=(argv[2]);
        inPath=(argv[1]);
    }
    if (argc<3) {
        cout << "Usage: filemover $inputFile $outputFile $threads $operationMode" << endl;
        cout << "operationMode: 1=compress; 2=encrypt; 3=compress+encrypt; 4=reverse" << endl;
        return 0;
    }

    try {inputFile inFile(&inPath,&operationMode,&threads);}
    catch (exception& e) {e.what(); exit(1);}; 

    inputFile inFile(&inPath,&operationMode,&threads);

    try {outputFile ofFile(&ofPath,&operationMode);}
    catch (exception& e) {e.what(); exit(1);}; 

    outputFile ofFile(&ofPath,&operationMode);

    #ifdef DEBUG
    cout << "inputFile: " << inFile.path << endl;
    cout << "outputFile: " <<  ofFile.path << endl;
    #endif

    inFile.startMoving(&threads,&ofFile);
    ofFile.glueChunks(&threads);

    //finish up with file system (c++17 filesystem/boost_filesystem)
    ofFile.setPermissions(&inFile);
    ofFile.setWriteTime(&inFile);
    
    #ifndef DEBUG
    fs::remove(inFile.getFilesystemPath());
    #endif
    return 0;
}
