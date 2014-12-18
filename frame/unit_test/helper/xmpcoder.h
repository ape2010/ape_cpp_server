#ifndef _XMP_CODER_H_
#define _XMP_CODER_H_

#include <stdint.h>
#include "buffer.h"

namespace protocol {
#define XMP_PACKAGE_MAX_SIZE   4096
#define XMP_EXTHEAD_MAX_SIZE   127

typedef enum {
    XMPTypeNone = 0X00,
    XMPTypeXTP = 0X01,
    XMPTypeSCP = 0X02
} EXmpType;
typedef enum {
    XMPTagNone = 0X00,
    XMPTagDatetime = 0X01,
    XMPTagCompressMethod = 0X02,
    XMPTagTransactionId = 0x03,
    XMPTagSessionState = 0x04,
    XMPTagKeepAlive = 0x05,
    XMPTagTradedate = 0x06,
    XMPTagHeartbeatTimeOut = 0x07
} EXmpTagType;

typedef struct stXmpHeader {
    uint8_t type;
    uint8_t extheaderlen;
    uint16_t bodylen;
}SXmpHeader;

typedef struct stXMPExtHeader {
    uint8_t tag;
    uint8_t taglen;
    uint8_t data[XMP_EXTHEAD_MAX_SIZE];
}SXmpExtHeader;

class CXmpEncoder {
 public:
    CXmpEncoder();
    CXmpEncoder(EXmpType type);
    void SetXmpType(EXmpType type);
    void SetExHeader(EXmpTagType tag, const void *data, int len);
    virtual void SetBody(const void *data, int len);

    const void *GetBuffer() const {return buf_.base();}
    int GetLen() const {return buf_.len();}

 private:
    CBuffer buf_;
};

class CXmpDecoder {
 public:
    CXmpDecoder(const void *data=NULL, int len=0);
    int Decode(const void *data, int len);
    const char *GetBody() const {return body_;}
    EXmpType GetXmpType() const {return (EXmpType)(header_.type);}
    EXmpTagType GetExtTag() const {return (EXmpTagType)(ext_header_.tag);}
    const char *GetExtData() const {return (const char *)(ext_header_.data);}
    int GetExtDataLen() const {return ext_header_.taglen;}

 private:
    SXmpHeader header_;
    SXmpExtHeader ext_header_;
    const char *body_;
};
}
#endif
