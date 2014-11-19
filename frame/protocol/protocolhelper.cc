#include "protocolhelper.h"
#include "httpsession.h"
#include "httpparser.h"
#include "thriftparser.h"

namespace ape {
namespace protocol {

int ParseAddr(const char *addr, EProtocolType *proto, char *ip, unsigned int *port) {
    char pro[32] = {0};
    if (3 != sscanf(addr, "%31[^:]://%[^:]:%u", pro, ip, port)) {
        return -1;
    }

    if (0 == strncasecmp("http", pro, 4)) {
        *proto = E_PROTOCOL_HTTP;
    } else if (0 == strncasecmp("thrift", pro, 4)) {
        *proto = E_PROTOCOL_THRIFT;
    } else {
        return -2;
    }
    return 0;
}


}
}
