#ifndef _APE_COMMON_HTTP_SESSION_H_
#define _APE_COMMON_HTTP_SESSION_H_

#include "session.h"
#include "httpmessage.h"
#include <deque>

namespace ape{
namespace net{

class CHttpSession : public CSession {
 public:
    typedef struct stResponseItem {
        unsigned int no;
        ape::message::SNetMessage *response;
        stResponseItem() : no(0), response(NULL) {};
        stResponseItem(unsigned int n) : no(n), response(NULL) {};
    }SResponseItem;
    CHttpSession() : dwseq(0) {}
    virtual void OnRead(ape::message::SNetMessage *msg);
    virtual void DoSendBack(void *para, bool close = false);
    virtual void Dump();
    virtual ~CHttpSession();

 private:
    unsigned int dwseq;
    std::deque<SResponseItem> request_deque_;
};

class HttpSessionFactory: public SessionFactory {
 public:
    HttpSessionFactory() {
        SessionFactory::RegisterFactory(ape::protocol::E_PROTOCOL_HTTP, this);
    }
    virtual CSession *CreateSession() {
        return new CHttpSession();
    }
    virtual ~HttpSessionFactory() {}
 private:
    static HttpSessionFactory http_session_factory_;
};

}
}
#endif

