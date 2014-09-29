#ifndef _APE_NET_NET_THREAD_HOLDER_H_
#define _APE_NET_NET_THREAD_HOLDER_H_
#include "netservice.h"
#include "sessioncallback.h"
#include <boost/thread/mutex.hpp>
#include <vector>
#include <string>

namespace ape {
namespace net {
class CTcpSocketServer;
class CSessionManager;
class CNetThreadHolder : public CNetServiceHolder {
public:
    CNetThreadHolder():ppthreads_(NULL), threadnum_(1), index_(0){}
    void Start(HandleFactory *f, int threadnum = 3);
    int StartServer(const std::vector<std::string> &addrs);
    void Stop();

    virtual CSessionCallBack *GetSessionCallBack();
    boost::asio::io_service *GetAcceptIoService();
    void GetThreadStatus(int &threadnum, int &deadthreadnum);
    void Dump();

private:
    CSessionManager **ppthreads_;
    std::vector<CTcpSocketServer *> servers_;
    int threadnum_;
    unsigned int index_;
};
}
}
#endif
