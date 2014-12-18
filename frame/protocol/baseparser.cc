#include "baseparser.h"
#include "loghelper.h"
#include "httpparser.h"
#include "thriftparser.h"
//#include "ftdparser.h"

namespace ape{
namespace protocol{

/** init static parser factory here, make sure the init sequence*/

ParserFactory *ParserFactory::factories_[E_PROTOCOL_ALL] = {NULL};
HttpParserFactory HttpParserFactory::http_parser_factory_;
ThriftParserFactory ThriftParserFactory::thrift_parser_factory_;
//FtdParserFactory FtdParserFactory::ftd_parser_factory_;

CBaseParser *ParserFactory::CreateParser(EProtocolType protocol) {
    if (protocol >= E_PROTOCOL_ALL || factories_[protocol] == NULL) {
        BS_XLOG(XLOG_ERROR, "ParserFactory::%s, bad protocol[%d]\n", __FUNCTION__, protocol);
        return NULL;
    }

    return factories_[protocol]->CreateParser();
}
void ParserFactory::RegisterFactory(EProtocolType protocol, ParserFactory*factory) {
    factories_[protocol] = factory;
}
void ParserFactory::Dump()
{
    for (int i = 0; i < E_PROTOCOL_ALL; ++i) {
        fprintf(stdout, "ParserFactory::%s, protocol[%d], factory[0X%0X]\n", __FUNCTION__, i, factories_[i]);
    }
}
}
}