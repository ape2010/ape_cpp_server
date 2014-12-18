#ifndef _APE_MESSAGE_HTTP_MESSAGE_EVENTS_H_
#define _APE_MESSAGE_HTTP_MESSAGE_EVENTS_H_

#include "events.h"

namespace ape {
namespace message {

typedef struct stHttpMessage : public SEventType<ape::protocol::E_PROTOCOL_HTTP, SNetMessage> {
    typedef struct stSCookie {
        std::string value;
        std::string path;
        std::string domain;
        time_t expires;
        stSCookie() :expires (0) {}
        stSCookie(const std::string &v, const std::string &p, const std::string &d, time_t e = 0) :
            value(v), path(p), domain(d), expires(e) {}
    }SCookie;
    typedef enum { HTTP_1_0 = 0, HTTP_1_1 = 1} EHttpVersion;

    bool keepalive;
    unsigned int requestno;
    EHttpVersion httpversion;
    std::string method;
    std::string url;
    std::string xforwordip;
    std::string body;
    boost::unordered_map<std::string, std::string> headers;
    boost::unordered_map<std::string, SCookie> cookies;

    stHttpMessage() : keepalive(false), requestno(0), httpversion(HTTP_1_0) {}
    virtual ~stHttpMessage(){}
    virtual unsigned int GetSequenceId(){return 0;}
    virtual bool IsOk() {return code == 200;}
    virtual std::string NoticeInfo();
    virtual void Dump();
    virtual std::string BriefInfo();

    void AddHeader(const std::string &key, const std::string &value);
    void SetCookie(const std::string &key, const std::string &value, const std::string &path, const std::string &domain, time_t e = 0);
}SHttpMessage;


}
}

#endif
