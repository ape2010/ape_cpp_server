#ifndef _FTD_CODER_H_
#define _FTD_CODER_H_

#include <stdint.h>
#include "buffer.h"

namespace protocol {
typedef enum {
    E_FTD_CHAIN_CONTINUE = 'C',
    E_FTD_CHAIN_LAST = 'L'
} EFtdChainType;

typedef enum {
    E_FTD_STREAM_DIALOG = 1,       /** 对话流*/
    E_FTD_STREAM_PRIVATE = 2,      /** 会员私有流*/
    E_FTD_STREAM_PUBLIC = 3,       /** 公共流*/
    E_FTD_STREAM_QUERY = 4,        /** 查询*/
    E_FTD_STREAM_USER = 5,         /** 交易员私有流*/
    E_FTD_STREAM_FORQUOTE = 6      /** 询价流*/
}EFtdStreamType;

typedef struct  stFtdHeader {
    uint8_t     version;            /** 版本号    1   二进制无符号整数。目前版本为1*/
    uint8_t     chain;              /** 报文链    1   ASCII码字符。*/
    uint16_t    streamtype;         /** 序列类别号  2   二进制无符号短整数。*/
    uint32_t    transactionid;      /**（TID）   FTD信息正文类型   4   二进制无符号整数。*/
    uint32_t    seqno;         /**（SeqNo） 序列号 4   二进制无符号整数。*/
    uint16_t    fieldcount;         /** 数据域数量  2   二进制无符号短整数。*/
    uint16_t    bodylen;            /** FTDC信息正文长度 2   二进制无符号短整数。以字节为单位。*/
    uint32_t    requestid;          /** 请求编号(由发送请求者维护，应答中会带回)  4 二进制无符号整数。*/
}SFtdHeader;

class CFtdEncoder {
 public:
    CFtdEncoder();
    CFtdEncoder(uint32_t tid, uint32_t requestid, uint32_t seqno, const void *body = NULL, int bodylen = 0);
    CFtdEncoder(uint32_t tid, uint32_t requestid, uint32_t seqno, EFtdStreamType streamtype=E_FTD_STREAM_DIALOG, EFtdChainType chain=E_FTD_CHAIN_LAST, uint8_t version=1, const void *body = NULL, int bodylen = 0);
    void SetVersion(uint8_t version);
    void SetChain(EFtdChainType chain);
    void SetStreamType(EFtdStreamType type);
    void SetTid(uint32_t tid);
    void SetSeqNo(uint32_t seqno);
    void SetSequestId(uint32_t requestid);
    void AddField(const void *data, uint16_t len);
    virtual void SetBody(const void *data, int len);

    const void *GetBuffer() const {return buf_.base();}
    int GetLen() const {return buf_.len();}

 private:
    CBuffer buf_;
};


}

#endif
