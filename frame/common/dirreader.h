#ifndef _APE_COMMON_DIR_READER_H
#define _APE_COMMON_DIR_READER_H

#include<stdio.h>
#ifndef WIN32
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <unistd.h>
#include <sys/stat.h>
#else
#include <windows.h>
#include <io.h>
#endif

#ifndef WIN32
#define  MAX_PATH 256
#endif

#include <string>
#include <vector>
namespace ape {
namespace common {
class CDirReader {
  public:
    CDirReader(const char* filter = NULL);
    virtual ~CDirReader();
  public:
    bool OpenDir(const char* path);
    bool GetFirstFilePath(char *filename);
    bool GetNextFilePath(char *filename);
  private:
    void ReadFiles(const char*path);
  private:
    std::string path_;
    std::string filter_;
    std::vector<std::string> filenames_;
    unsigned int currentindex_;
};
}
}
#endif
