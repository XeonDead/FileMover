#include <iostream>
#include <chrono>
#include <filesystem>
#include <thread>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <ios>
#include <iterator>
#include <vector>

//#define DEBUG

using namespace std;
namespace fs = std::filesystem; //this is c++17 at work, but boost_filesystem is essentially same

class File {
    public:
        string Path;
        fs::path FsPath;
    File(string Path) {
        File.FsPath = fs::u8path(Path);
    }
    File::MakeChunk();
}

class InputFile: public File {
    public:
        unsigned int Parameters:3;
        fs::perms FilePermissions;
    InputFile(string Path) {
        InputFile.FilePermissions = fs::status(Path).permissions();
    }
}

class OutputFile: public File{
    public:
    OutputFile::GlueChunks();
}

string InvertFile(string *File) {
    ifstream input_file(File->c_str(), ios::binary);
    ofstream output_file("tmpReversedFile", ios::binary);
    istreambuf_iterator<char> input_begin(input_file);
    istreambuf_iterator<char> input_end;
    ostreambuf_iterator<char> output_begin(output_file);
    vector<char> input_data(input_begin, input_end);

    #ifdef DEBUG
    cout << "Reversing input file" << endl;
    #endif

    reverse_copy(input_data.begin(), input_data.end(), output_begin);

    return "tmpReversedFile";
}

string PreprocessFile(string *OrigFile, int Operation){
    string tmpCompressedFile; string tmpEncryptedFile;
    #ifdef DEBUG
    cout << "Starting preprocess with mode " << Operation << endl;
    #endif
    switch (Operation) {
        //compress+encrypt stub
        case(3):tmpCompressedFile=PreprocessFile(OrigFile,1);
                tmpEncryptedFile=PreprocessFile(&tmpCompressedFile,2);
                return tmpEncryptedFile;
        //compress stub
        case(1):return *OrigFile;
        //encrypt stub
        case(2):return *OrigFile;
        //invert
        case(4):return InvertFile(OrigFile);
        default: return *OrigFile;
    }
}

string IntToString(int Int){
    //this function is written to purely translate int to string without weak conversions (C-like itoa)
    string ResString;
    stringstream temp_str;
    temp_str<<Int;
    ResString = temp_str.str();
    return ResString;
}

void MakeChunk(const int* ChunkNum, int* ChunkSize, string* InputFile, string* OutputFile) {
    //this function grabs a numbered chunk from InputFile and creates it at the destination
    #ifdef DEBUG
    cout << "MakeChunk " << *ChunkNum << endl;
    #endif
    char *buffer = new char[*ChunkSize];
    ofstream ChunkFile;
    ifstream infile(InputFile->c_str());
    string Chunk=*OutputFile;
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

void GlueChunks (string* OutputFile, int* ThreadCount) {
    //this function glues the resulting chunks to one output file
    #ifdef DEBUG
    cout << "Starting GlueChunks" << endl;
    #endif
    string fileName;
    ofstream OutFile;
    OutFile.open(*OutputFile, ios::out | ios::binary);

    if (OutFile.is_open()) {
        #ifdef DEBUG
        cout << *OutputFile << " opened to write" << endl;
        #endif

        for (int i=0;i<*ThreadCount;i++)
        {
            // Build the filename
            fileName.clear();
            fileName.append(*OutputFile);
            fileName.append(".");
            fileName.append(IntToString(i));

            // Open chunk to read
            ifstream fileInput;
            fileInput.open(fileName.c_str(), ios::in | ios::binary);
            #ifdef DEBUG
            cout << fileName << " opened to read" << endl;
            #endif

            // If chunk opened successfully, read it and write it to the output file.
            if (fileInput.is_open() && fs::file_size(fs::u8path(fileName))!=0) {
                int ChunkSize = fs::file_size(fs::u8path(fileName));
                char *inputBuffer = new char[ChunkSize];
                // After a little brainstorm, using actual chunk size is a better idea
                fileInput.read(inputBuffer,ChunkSize);
                OutFile.write(inputBuffer,ChunkSize);
                delete[](inputBuffer);
                fileInput.close();
            }
        }
        // Close output file.
        OutFile.close();
        #ifdef DEBUG
        cout << "File assembly complete!" << endl;
        #endif
    }
    else { cout << "Error: Unable to open file for output." << endl; }
}

int main( int argc , char *argv[ ] ) {
        //initialize empty paths
    string InputFile = "", AttribFile = "", OutputFile = "";
        //mutex for thread-locking
    mutex mtx;
        //initial thread count and chunk size
    int ThreadCount=5;int ChunkSize=0;int OperationMode=0;
        //populate them from command-line
    if (argc<2){
        cout << "Usage: filemover $InputFile $OutputFile $ThreadCount $OperationMode" << endl;
        cout << "OperationMode: 1=compress; 2=encrypt; 3=compress+encrypt; 4=reverse" << endl;
        return 0;
    }
    if (argc > 2)
    {
        InputFile = argv[1];
        OutputFile = argv[2];
    }
        //Nothing to do if we're not given an output file
    if (OutputFile=="") {cout << "Only input file specified" << endl; return 1;}
    if (argc > 3) {ThreadCount=atoi(argv[3]);};
    if (argc > 4) {OperationMode=atoi(argv[4]);};
    #ifdef DEBUG
    cout << "InputFile: " << InputFile << endl;
    cout << "OutputFile: " <<  OutputFile << endl;
    #endif
        //If OperationMode is not 0, we need to take original file's attributes instead of the new InputFile
    AttribFile=InputFile;
    InputFile=PreprocessFile(&InputFile,OperationMode);
    vector<thread> threadsPool(ThreadCount);
    fs::path InputPath = fs::u8path(AttribFile);
    fs::path OutputPath = fs::u8path(OutputFile);
        //this is needed since we can accidentally end up with 0 data in chunks
    if (ThreadCount>(fs::file_size(InputPath)/2)) {cout << "Chunks too small to contain any data" << endl;; return 1;}; 
    if ((fs::file_size(InputPath)%ThreadCount)==0) 
    {ChunkSize = (fs::file_size(InputPath)/ThreadCount); } 
    else {ChunkSize = (fs::file_size(InputPath)/ThreadCount)+1; }
        // If file size does not evenly divide in our chunk/thread count, we add one more byte to the chunk size 
        // this is taken from consideration where if file size has a modulus after division we can be certain that if chunk size will be bigger it will cover
        // the entire file so if we just extend the chunk a little it will contain the file entirely
    #ifdef DEBUG
    cout << "Size " << ChunkSize << endl;
    #endif
    for (int i=0;i<ThreadCount;i++) {
        //critical section - mutex to avoid race condition and multiple creation of the same chunks
        mtx.lock();
        #ifdef DEBUG
        cout << "Thread " << i << endl;
        #endif
        threadsPool[i] = thread(&MakeChunk,&i,&ChunkSize,&InputFile,&OutputFile);
        threadsPool[i].join();
        mtx.unlock();
    }
        //we don't need any tmp files past this point so we can safely remove them
    fs::remove("tmpCompressedFile");
    fs::remove("tmpEncryptedFile");
    fs::remove("tmpReversedFile");
        //we need the exitfile since we have chunks to merge to it
    ofstream offile(OutputFile);
    GlueChunks(&OutputFile,&ThreadCount);
        //finish up with file system (c++17 filesystem/boost_filesystem)
    fs::permissions(OutputPath, fs::status(InputPath).permissions(), fs::perm_options::replace);
    fs::last_write_time(OutputPath,fs::last_write_time(InputPath));
    #ifndef DEBUG
    fs::remove(InputPath);
    #endif
    for (int i=0;i<ThreadCount;i++)
    {
        //clean up
        string ChunkName;
        ChunkName.clear();
        ChunkName.append(OutputFile);
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
