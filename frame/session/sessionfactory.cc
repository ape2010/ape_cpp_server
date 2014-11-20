#include "session.h"
#include "loghelper.h"
#include "httpsession.h"

namespace ape{
namespace net{

SessionFactory *SessionFactory::factories_[ape::protocol::E_PROTOCOL_ALL] = {NULL};
HttpSessionFactory HttpSessionFactory::http_session_factory_;

CSession *SessionFactory::CreateSession(ape::protocol::EProtocolType protocol) {
    if (protocol >= ape::protocol::E_PROTOCOL_ALL) {
        BS_XLOG(XLOG_ERROR, "SessionFactory::%s, bad protocol[%d]\n", __FUNCTION__, protocol);
        return NULL;
    }

    return  factories_[protocol] != NULL ? factories_[protocol]->CreateSession() : new CSession();
}
void SessionFactory::RegisterFactory(ape::protocol::EProtocolType protocol, SessionFactory *factory) {
    factories_[protocol] = factory;
}

}
}
