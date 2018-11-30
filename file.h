#ifndef CB_FILE_H
#define CB_FILE_H
#endif // CB_FILE_H

struct Parameters {
    bool toCompress = false;
    bool toEncrypt = false;
    bool toInverse = false;
};

class File {
    private:
        std::string path_;
        boost::filesystem::path fspath_;
        Parameters parameters_;
        int fileSize_; int chunkSize_;
    public:

        File(std::string path, int InitParameters, int threads);

        void setPath(const std::string path);
        std::string getPath() const;
        boost::filesystem::path getFilesystemPath() const;
        
        boost::filesystem::perms getPermissions() const;
        void setPermissions(const boost::filesystem::perms permissions);

        std::time_t getWriteTime() const;
        void setWriteTime(const std::time_t writeTime);
        
        int getChunkSize() const;
        int getFileSize() const;

        int startMoving(const int threads, const File* outputFile);
        static int makeChunk(const File* inputFile, int chunkNum, const File* outputFile);
        //^^^^ invokable issue. might not be able to create threads if there's no object - compiler complaint
        int glueChunks(const int threads);
};
