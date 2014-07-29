#ifndef _APE_COMMON_SO_LODER_H_
#define _APE_COMMON_SO_LODER_H_

#include <string>
#include <vector>

namespace ape {
namespace common {

typedef void *(*create_obj)(int nthreadid);
typedef void  (*destroy_obj)(void*);

typedef struct stSoUnit {
    std::string strsoname;
    void *phandle;
    create_obj pfuncreate;
    destroy_obj pfundestroy;

    stSoUnit(const std::string &strname, create_obj funcreate, destroy_obj fundestroy) {
        phandle = NULL;
        strsoname = strname;
        pfuncreate = funcreate;
        pfundestroy = fundestroy;
    }
    stSoUnit():phandle(NULL),pfuncreate(NULL),pfundestroy(NULL){}
}SSoUnit;

template<typename Interface>
struct SSoClassInfo {
    std::string strsoname;
    Interface* pobj;
    destroy_obj pfundestroy;

    SSoClassInfo(const std::string &strname, Interface* obj, destroy_obj fundestroy) {
        strsoname = strname;
        pobj = obj;
        pfundestroy = fundestroy;
    }
    SSoClassInfo():pobj(NULL),pfundestroy(NULL){}
};


class CSoLoader
{
public:
    int Load(const std::string &strdir, const std::string &strfilter, std::vector<SSoUnit> &vecunit);

private:    
    int GetSoUnit(const char *szsoname, SSoUnit &sounit);
};
}
}
#endif
