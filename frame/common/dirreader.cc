#include "dirreader.h"
#include <cstring>
#include "loghelper.h"

namespace ape {
namespace common {
CDirReader::CDirReader(const char* filter) : currentindex_(0) {
    filter_ = (filter == NULL || filter[0] == 0) ? "*" : filter;
}
CDirReader::~CDirReader(){ }
bool CDirReader::GetNextFilePath(char *filename) {
    if (filenames_.empty() || currentindex_ >= filenames_.size())
        return false;
    snprintf(filename, 255, "%s", filenames_.at(currentindex_++).c_str());
    return true;
}
bool CDirReader::GetFirstFilePath(char *filename) {
    if (filenames_.empty())
        return false;
    currentindex_ = 0;
    snprintf(filename, 255, "%s", filenames_.at(currentindex_++).c_str());
    return true;
}
bool CDirReader::OpenDir(const char* path) {
    if (access(path,0) < 0) {
        return false;
    }
    ReadFiles(path);
    return true;
}
void CDirReader::ReadFiles(const char *path) {
    struct dirent *pdirent;
    struct stat statbuf;
    DIR *dir = opendir(path);
    if (NULL == dir) {
        BS_XLOG(XLOG_WARNING,"CDirReader::%s, open dir error. path[%s]\n", __FUNCTION__, path);
        return;
    }

    while (NULL != (pdirent = readdir(dir))) {
        if ((0 == strcmp(pdirent->d_name, ".")) || (0 == strcmp(pdirent->d_name, "..")))
            continue;

        std::string filepath = path;
        if (filepath.at(filepath.length()-1) != '/') {
            filepath.append("/");
        }
        filepath.append(pdirent->d_name);
        if(0 > stat(filepath.c_str(), &statbuf)) {
            BS_XLOG(XLOG_WARNING, "CDirReader::%s.   stat file error: %s\n", __FUNCTION__, filepath.c_str());
            continue;
        }

        if(S_ISDIR(statbuf.st_mode)) {
            ReadFiles(filepath.c_str());
        } else {
            //BS_XLOG(XLOG_TRACE,"CDirReader::%s.   fileName[%s]\n",__FUNCTION__, filepath.c_str());
            if (filter_.empty() || 0 == fnmatch(filter_.c_str(), pdirent->d_name, FNM_PATHNAME|FNM_PERIOD)) {
                //BS_XLOG(XLOG_TRACE,"CDirReader::%s.   matched fileName[%s]\n",__FUNCTION__, filepath.c_str());
                filenames_.push_back(filepath);
            }
        }
    }
    closedir(dir);
}
}
}
