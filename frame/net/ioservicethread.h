#ifndef _APE_NET_IO_THREAD_H_
#define _APE_NET_IO_THREAD_H_
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "timermanager.h"
#include "threadtimer.h"
#include "handleralloc.h"

namespace ape {
namespace net {

class CIoServiceThread : public ape::common::CTimerManager {
public:
    CIoServiceThread();
    CIoServiceThread(int threadid);
    ~CIoServiceThread();
    void Start();
    void Stop();
    int GetThreadId(){return threadid_;}
    void GetSelfCheck(bool &isalive);

    virtual boost::asio::io_service *GetIoService(){return &ioservice_;}
    virtual void StartInThread();
    virtual void StopInThread();
    virtual void Dump();

private:
    void InitInner();
    void DoTimer();
    void DoSelfCheck();

private:
    int threadid_;
    boost::asio::io_service ioservice_;
    boost::asio::io_service::work *work_;
    boost::thread thread_;

    volatile bool isalive_;
    boost::asio::deadline_timer dealline_timer_;
    ape::common::CHandlerAllocator alloc_timer;

    ape::common::CThreadTimer tm_selfcheck_;
};
}
}
#endif

