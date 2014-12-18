#ifndef _FIELD_BUFFER_H_
#define _FIELD_BUFFER_H_

#include <stdint.h>
#include <string>
#include "buffer.h"

namespace protocol {

typedef struct stFieldHeader
{
    uint16_t fieldid;
    uint16_t len;
}SFieldHeader;

class CField {
 public:
    CField();
    CField(uint16_t fieldid);
    void SetFieldId(uint16_t fieldid);
    void AddValue(int32_t value);
    void AddUint16(uint16_t value);
    void AddUint8(uint8_t value);
    void AddValue(const std::string &value, int32_t maxlen);
    void AddValue(const char *value, int32_t maxlen);
    void AddValue(const void *value, int32_t len, int32_t maxlen);

    const void *GetBuffer() const {return buf_.base();}
    int GetLen() const {return buf_.len();}

 private:
    CBuffer buf_;
};

}

#endif
