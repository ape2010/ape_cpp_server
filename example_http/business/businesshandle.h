#ifndef _APE_TEST_BUSINESS_HANDLE_H_
#define _APE_TEST_BUSINESS_HANDLE_H_
#include <ape/events.h>
#include <ape/netservice.h>
#include <vector>
#include <string>

namespace ape {
namespace testhttp {

class CBusinessHandle : public ape::net::CHandle {
 public:
    typedef void (CBusinessHandle::*DealUrlFunc)(ape::message::SHttpMessage *, const std::string &);
    CBusinessHandle();
    virtual ~CBusinessHandle() {}
    virtual void StartInThread(int threadid, ape::net::CNetService *service, ape::common::CTimerManager *owner);
    virtual void StopInThread(int threadid) {}
    virtual void OnEvent(int threadid, void *event);
    void Dump();

 private:
    void DealThriftResponse(ape::message::SThriftMessage *message);
    void DealHttpRequest(ape::message::SHttpMessage *message);
    void DoSendHttpResponse(ape::message::SHttpMessage *request, int code, const std::string &body);
    void DoTestThriftRequest(ape::message::SHttpMessage *message);
 private:
    int threadid_;
    ape::net::CNetService *service_;
    ape::common::CTimerManager *timerowner_;
    
    uint32_t smseqid_;
};
}
}
#endif
