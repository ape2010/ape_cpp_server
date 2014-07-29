#ifndef _APE_PTOTOCOL_PROTOCOL_H_
#define _APE_PTOTOCOL_PROTOCOL_H_

namespace ape {
namespace protocol {
typedef enum {
    PROTOCOL_ERROR = 0,
    PROTOCOL_HTTP,
    PROTOCOL_THRIFT,
    PROTOCOL_RAPID,
    PROTOCOL_WEAK
}EProtocol;
}
}

#endif
