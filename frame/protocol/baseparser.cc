#include "baseparser.h"
#include "loghelper.h"

namespace ape{
namespace protocol{

ParserFactory *ParserFactory::factories_[E_PROTOCOL_ALL] = {NULL};
CBaseParser *ParserFactory::CreateParser(EProtocolType protocol) {
    if (protocol >= E_PROTOCOL_ALL || factories_[protocol] == NULL) {
        BS_XLOG(XLOG_ERROR, "ParserFactory::%s, bad protocol[%d]\n", __FUNCTION__, protocol);
        return NULL;
    }

    return factories_[protocol]->CreateParser();
}
void ParserFactory::RegisterFactory(EProtocolType protocol) {
    factories_[protocol] = this;
}

}
}