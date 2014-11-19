#ifndef _APE_PTOTOCOL_PROTOCOL_H_
#define _APE_PTOTOCOL_PROTOCOL_H_

namespace ape {
namespace protocol {
typedef enum { E_TCP = 0, E_UDP, E_SOCKET } ESocketType;

typedef enum {
    E_PROTOCOL_ERROR = 0,
    E_PROTOCOL_HTTP,
    E_PROTOCOL_THRIFT,
    E_PROTOCOL_FTD,
    E_PROTOCOL_ALL
}EProtocolType;

}
}

#endif
