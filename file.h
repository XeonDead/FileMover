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
    unsigned long fileSize_; unsigned long chunkSize_;
  public:
    File(std::string path, int InitParameters, unsigned long threads);

    void setPath(std::string path);
    std::string getPath() const;
    boost::filesystem::path getFilesystemPath() const;
        
    boost::filesystem::perms getPermissions() const;
    void setPermissions(boost::filesystem::perms permissions);

    std::time_t getWriteTime() const;
    void setWriteTime(std::time_t writeTime);
        
    unsigned long getChunkSize() const;
    unsigned long getFileSize() const;

    int startMoving(unsigned long threads, const File* outputFile);
    static int makeChunk(const File* inputFile, int chunkNum, const File* outputFile);
    //^^^^ invokable issue. might not be able to create threads if there's no object - compiler complaint
    int glueChunks(unsigned long threads);
};
