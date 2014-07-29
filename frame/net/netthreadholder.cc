#include "netthreadholder.h"
#include "tcpsocketserver.h"
#include "sessionmanager.h"
#include <boost/bind.hpp>
#include "loghelper.h"
#include <boost/date_time.hpp>
#include <dlfcn.h>

namespace ape {
namespace net {
void CNetThreadHolder::Start(HandleFactory *f, int threadnum) {
    BS_XLOG(XLOG_DEBUG,"CNetThreadHolder::Start[%d]...\n",threadnum);
    threadnum_ = threadnum + 1;
    ppthreads_ = new CSessionManager *[threadnum_];

    for (int i = 0; i < threadnum_; ++i) {
        CSessionManager *thread = new  CSessionManager(f, i);
        ppthreads_[i] = thread;
        thread->Start();
    }
}
int CNetThreadHolder::StartServer(const std::vector<std::string> &addrs) {
    std::vector<std::string>::const_iterator itr = addrs.begin();
    for(; itr != addrs.end(); ++itr) {
        CTcpSocketServer *server = new CTcpSocketServer(*(GetAcceptIoService()), this);
        servers_.push_back(server);
        if (0 != server->StartServer(*itr)) {
            return -1;
        }
    }
    return 0;
}
void CNetThreadHolder::Stop() {
    BS_XLOG(XLOG_DEBUG,"CNetThreadHolder::Stop\n");
    for(std::vector<CTcpSocketServer *>::iterator itr = servers_.begin(); itr != servers_.end(); ++itr) {
        (*itr)->StopServer();
        //delete *itr;
    }
    for(int i = 0; i < threadnum_; ++i) {
        CSessionManager *thread = ppthreads_[i];
        thread->Stop();
        delete thread;
    }
    for(std::vector<CTcpSocketServer *>::iterator itr = servers_.begin(); itr != servers_.end(); ++itr) {
        delete *itr;
    }
    servers_.clear();
    
    delete[] ppthreads_;
}

boost::asio::io_service *CNetThreadHolder::GetAcceptIoService() {
    return ppthreads_[threadnum_-1]->GetIoService();
}
CNetService *CNetThreadHolder::GetNetService() {
    if (threadnum_ <= 1) {
        return ppthreads_[0];
    }
    int i = (index_++) % (threadnum_ - 1);
    return ppthreads_[i];
}

void CNetThreadHolder::GetThreadStatus(int &threadnum, int &deadthreadnum) {
    BS_XLOG(XLOG_DEBUG,"CNetThreadHolder::%s..\n", __FUNCTION__);
    threadnum = threadnum_;
    for(int i = 0; i < threadnum_; ++i) {
        CSessionManager *thread = ppthreads_[i];
        bool isalive;
        thread->GetSelfCheck(isalive);
        if(!isalive)
            deadthreadnum++;
    }
}

void CNetThreadHolder::Dump() {
    for(int i = 0; i < threadnum_; ++i) {
        CSessionManager *thread = ppthreads_[i];
        BS_XLOG(XLOG_DEBUG,"\n\n\n=====DUMP[%d], manager[%p]\n", i, thread);
        thread->Dump();
    }
}
}
}

