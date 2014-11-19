#ifndef _APE_PTOTOCOL_PROTOCOL_HELPER_H_
#define _APE_PTOTOCOL_PROTOCOL_HELPER_H_

#include "protocol.h"
#include "session.h"

namespace ape {
namespace protocol {
int ParseAddr(const char *addr, EProtocolType *proto, char *ip, unsigned int *port);

}
}

#endif
