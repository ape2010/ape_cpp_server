#include "soloader.h"
#include "loghelper.h"
#include "dirreader.h"
#include <dlfcn.h>

namespace ape {
namespace common {
#ifdef WIN32
    #define CST_PATH_SEP    "\\"
#else
    #define CST_PATH_SEP    "/"    
#endif

#ifndef MAX_PATH
#define MAX_PATH 256
#endif


int CSoLoader::Load(const std::string &strdir, const std::string &strfilter, std::vector<SSoUnit> &vecunit) {
    BS_XLOG(XLOG_DEBUG,"CSoLoader::%s, dir[%s],filter[%s]\n",__FUNCTION__,strdir.c_str(),strfilter.c_str());
    CDirReader oDirReader(strfilter.c_str());
    if(!oDirReader.OpenDir(strdir.c_str())) {
        BS_XLOG(XLOG_DEBUG,"CSoLoader::%s,open dir error[%s]\n",__FUNCTION__,strdir.c_str());
        return -1;
    }

    char szfilename[MAX_PATH] = {0};

    if(!oDirReader.GetFirstFilePath(szfilename)) {
        BS_XLOG(XLOG_DEBUG,"CSoLoader::%s,GetFirstFilePath error, dir[%s]\n",__FUNCTION__,strdir.c_str());
        return 0;
    }
    
    do {
        SSoUnit sounit;
        if(0 != GetSoUnit(szfilename,sounit)) {
            BS_XLOG(XLOG_ERROR,"CSoLoader::%s,GetSoUnit[%s] error\n",__FUNCTION__,szfilename);
            continue;
        }
        vecunit.push_back(sounit);
    }while(oDirReader.GetNextFilePath(szfilename));

    return 0;
}


int CSoLoader::GetSoUnit(const char *szsoname, SSoUnit &sounit) {
    BS_XLOG(XLOG_DEBUG,"CSoLoader::%s, SoName[%s]\n",__FUNCTION__,szsoname);

    void *handle = NULL;
    if(NULL == (handle = dlopen(szsoname, RTLD_LAZY))) {
        BS_XLOG(XLOG_ERROR,"CSoLoader::%s,dlopen[%s] error[%s]\n", __FUNCTION__,szsoname,dlerror());
        return -1;
    }

    create_obj create_unit = (create_obj)dlsym(handle, "create");
    char* dlsym_error = dlerror();
    if (dlsym_error) {
        BS_XLOG(XLOG_ERROR,"CSoLoader::%s,dlsym create[%s] error[%s]\n",__FUNCTION__,szsoname,dlsym_error);
        return -1;
    }

    destroy_obj destroy_unit = (destroy_obj)dlsym(handle, "destroy");
    dlsym_error = dlerror();
    if (dlsym_error) {
        BS_XLOG(XLOG_ERROR,"CSoLoader::%s,dlsym destroy[%s] error[%s]\n",__FUNCTION__,szsoname,dlsym_error);
        return -1;
    }

    sounit.strsoname = szsoname;
    sounit.phandle = handle;
    sounit.pfuncreate = create_unit;
    sounit.pfundestroy = destroy_unit;

    return 0;
}
}
}
