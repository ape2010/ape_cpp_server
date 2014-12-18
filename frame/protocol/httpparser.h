#ifndef _APE_HTTP_PARSER_H_
#define _APE_HTTP_PARSER_H_
#include "baseparser.h"
#include "httpmessage.h"
#include "loghelper.h"
#include <list>

namespace ape{
namespace protocol{

class CHttpParser : public CBaseParser{
 public:
    CHttpParser();
    virtual ape::message::SNetMessage *CreateMessage() {return new ape::message::SHttpMessage;}
    virtual ape::message::SNetMessage *CreateHeartBeatMessage(ape::message::SNetMessage::SMessageDirection direction = ape::message::SNetMessage::E_Request);
    virtual const char *Decode(const char *buf, int len, ape::message::SNetMessage *e);
    virtual int Encode(const ape::message::SNetMessage *e, ape::common::CBuffer *out);
    virtual void Release() {delete this;}
    virtual ~CHttpParser() {}
 private:
    void ParseHeader(const std::string &key, const std::string &value, ape::message::SHttpMessage *message);
    int  EncodeRequest(const ape::message::SHttpMessage *msg, ape::common::CBuffer *out);
    int  EncodeResponse(const ape::message::SHttpMessage *msge, ape::common::CBuffer *out);
    void DecodeCookie(const char *buf, int len, ape::message::SHttpMessage *message);
 private:
    static std::map<std::string, std::string> sm_default_header_;
    static std::map<std::string, std::string> sm_default_request_header_;
    std::list<std::string> default_header_list_;
    int contentlen_;
    bool ischunked_;
};

class HttpParserFactory: public ParserFactory {
 public:
    HttpParserFactory() {
        ParserFactory::RegisterFactory(E_PROTOCOL_HTTP, this);
    }
    virtual CBaseParser *CreateParser() {
        return new CHttpParser();
    }
    virtual ~HttpParserFactory() {}
 private:
    static HttpParserFactory http_parser_factory_;
};
}
}
#endif

