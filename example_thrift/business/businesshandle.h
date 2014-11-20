#ifndef _APE_MONITOR_BUSINESS_HANDLE_H_
#define _APE_MONITOR_BUSINESS_HANDLE_H_
#include <ape/events.h>
#include <ape/thriftmessage.h>
#include <ape/netservice.h>
#include <vector>
#include <string>

namespace ape {
namespace testthrift {
class CBusinessHandle : public ape::net::CHandle {
 public:
    CBusinessHandle();
    virtual ~CBusinessHandle() {}
    virtual void StartInThread(int threadid, ape::net::CNetService *service, ape::common::CTimerManager *owner);
    virtual void StopInThread(int threadid) {}
    virtual void OnEvent(int threadid, void *event);
    void Dump();
 private:
    void DealThriftRequest(ape::message::SThriftMessage *message);
 private:
    int threadid_;
    ape::net::CNetService *service_;
    ape::common::CTimerManager *timerowner_;
};
}
}
#endif
