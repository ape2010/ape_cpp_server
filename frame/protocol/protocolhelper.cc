#include "protocolhelper.h"
#include "httpsession.h"
#include "httpparser.h"
#include "thriftparser.h"

namespace ape {
namespace protocol {

int ParseAddr(const char *addr, EProtocolType *proto, char *ip, unsigned int *port) {
    char pro[32] = {0};
    int ret = sscanf(addr, "%31[^:]://%[^:]:%u", pro, ip, port);
    if (3 != ret && 2 != ret) {
        return -1;
    }

    if (0 == strncasecmp("http", pro, 4)) {
        if (ret == 2) {
            *port = 80;
        }
        *proto = E_PROTOCOL_HTTP;
    } else if (0 == strncasecmp("thrift", pro, 6)) {
        *proto = E_PROTOCOL_THRIFT;
    } else if (0 == strncasecmp("ftd", pro, 3)) {
        *proto = E_PROTOCOL_FTD;
    } else {
        return -2;
    }
    return 0;
}


}
}
