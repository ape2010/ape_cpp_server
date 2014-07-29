#ifndef _APE_PTOTOCOL_PROTOCOL_HELPER_H_
#define _APE_PTOTOCOL_PROTOCOL_HELPER_H_

#include "protocol.h"
#include "session.h"

namespace ape {
namespace protocol {
int ParseAddr(const std::string &addr, EProtocol *proto, char *ip, unsigned int *port);
ParserFactory *GetParserFactory(EProtocol p);
ape::net::CSession *CreateSession(EProtocol p);
}
}

#endif
