#include "protocolhelper.h"
#include "httpsession.h"
#include "httpparser.h"
#include "thriftparser.h"

namespace ape {
namespace protocol {

int ParseAddr(const std::string &addr, EProtocol *proto, char *ip, unsigned int *port) {
    char pro[32] = {0};
    if (3 != sscanf(addr.c_str(), "%31[^:]://%[^:]:%u", pro, ip, port)) {
        return -1;
    }
    
    if (0 == strncasecmp("http", pro, 4)) {
        *proto = PROTOCOL_HTTP;
    } else if (0 == strncasecmp("domain", pro, 4)) {
        *proto = PROTOCOL_HTTP;
    } else if (0 == strncasecmp("thrift", pro, 4)) {
        *proto = PROTOCOL_THRIFT;
    } else {
        return -2;
    }
    return 0;
}

ParserFactory *GetParserFactory(EProtocol p) {
    static HttpParserFactory http_factory;
    static ThriftParserFactory thrift_factory;
    switch (p) {
      case PROTOCOL_HTTP :
        return &http_factory;
      case PROTOCOL_THRIFT :
        return &thrift_factory;
      default:
        return NULL;
    }
}
ape::net::CSession *CreateSession(EProtocol p) {
    switch (p) {
      case PROTOCOL_HTTP :
        return new ape::net::CHttpSession();
      default:
        return new ape::net::CSession();
    }
}

}
}
