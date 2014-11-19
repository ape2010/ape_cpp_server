#ifndef _APE_COMMON_THRIFT_PARSER_H_
#define _APE_COMMON_THRIFT_PARSER_H_
#include "baseparser.h"

namespace ape{
namespace protocol{

class CThriftParser : public CBaseParser{
 public:
    CThriftParser();
    virtual ape::message::SNetMessage *CreateMessage() {return new ape::message::SThriftMessage;}
    virtual ape::message::SNetMessage *CreateHeartBeatMessage(ape::message::SNetMessage::SMessageType type = ape::message::SNetMessage::E_Request);
    virtual const char *Decode(const char *buf, int len, ape::message::SNetMessage *e);
    virtual int Encode(const ape::message::SNetMessage *e, ape::common::CBuffer *out);
    virtual void Release() {delete this;}
};

class ThriftParserFactory: public ParserFactory {
 public:
    ThriftParserFactory() {
        RegisterFactory(E_PROTOCOL_HTTP);
    }
    virtual CBaseParser *CreateParser() {
        return new CThriftParser();
    }
    virtual ~ThriftParserFactory() {}
 private:
    static ThriftParserFactory thrift_parser_factory_;
};
}
}
#endif

