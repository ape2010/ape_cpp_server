#ifndef _APE_COMMON_COH_THREAD_
#define _APE_COMMON_COH_THREAD_
#include <ape/netthreadholder.h>
#include <ape/netservice.h>
#include <ape/events.h>
#include <vector>
#include <string>

namespace ape {
namespace testhttp {

class CohThread : public ape::net::CHandle {
 public:
    typedef enum{FUNC_HELP = 0, FUNC_ALL}EFuncType;
    typedef std::string (CohThread::*DealCmdFunc)(ape::message::SHttpMessage *);
    typedef struct stFuncInfo {
        EFuncType type; 
        std::string desc;
        stFuncInfo(){}
        stFuncInfo(EFuncType t, const std::string &s) : type(t), desc(s){}
    }SFuncInfo;
 public:
    static CohThread *GetInstance();
    virtual ~CohThread();
    int StartServer(const std::string &addr);
    void Stop();
    void InitUrl(const std::string &errorrurl, const std::string &actionurl);
    void OnErrorReport(const std::string &info);
    void OnActionReport(const std::string &info);
 public:
    virtual void StartInThread(int threadid, ape::net::CNetService *service, ape::common::CTimerManager *owner);
    virtual void StopInThread(int threadid) {}
    virtual void OnEvent(int threadid, void *event);
    virtual void Release() {}
 public:
    typedef struct stUrlInfo {
        std::string addr;
        std::string name;
        std::string path;
    }SUrlInfo;
 private:
    CohThread();    
    void DealCohCmd(ape::message::SHttpMessage *message);
    std::string DoHelpInfo(ape::message::SHttpMessage *message);
 private:
    static CohThread * sm_instance_;
    ape::net::CNetThreadHolder holder_;
    ape::net::CNetService *service_;
    std::map<std::string, SFuncInfo> mapcmd_;
    DealCmdFunc funcmapping_[FUNC_ALL];

    SUrlInfo error_url_;
    SUrlInfo action_url;
};
class CohHandleFactory : public ape::net::HandleFactory {
 public:
    virtual ape::net::CHandle* NewHandler() {
        return CohThread::GetInstance();
    }
    virtual ~CohHandleFactory() {}
};
}
}
#endif
