#ifndef _COMPRESS_CODER_H_
#define _COMPRESS_CODER_H_

#include <stdint.h>
#include "buffer.h"
namespace protocol {
#define XMP_PACKAGE_MAX_SIZE   4096
#define XMP_EXTHEAD_MAX_SIZE   127

typedef enum {
    E_FTDC_TYPE = 0x01
}EProtocolType;

typedef struct stCompressHeader {
    uint8_t type;    /**< 标示上层协议的协议ID */
    uint8_t method;  /**< 压缩算法代码 */
}SCompressHeader;
typedef enum {E_COMPRESS_NONE = 0X00, E_COMPRESS_ZERO = 0X03} ECompressMethod;

class CCompressEncoder {
 public:
    CCompressEncoder(ECompressMethod method=E_COMPRESS_NONE, EProtocolType type=E_FTDC_TYPE, const void *body = NULL, int bodylen = 0);
    virtual void SetBody(const void *data, int len);

    const void *GetBuffer() const {return buf_.base();}
    int GetLen() const {return buf_.len();}

 private:
    CBuffer buf_;
};


class CCompressDecoder {
 public:
    CCompressDecoder(const void *data=NULL, int len=0);
    int Decode(const void *data, int len);
    const char *GetBody() const {return body_;}

 private:
    SCompressHeader header_;
    const char *body_;
    CBuffer buf_;
};

}
#endif
