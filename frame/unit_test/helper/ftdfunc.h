#include "ftdcoder.h"
#include "fieldbuffer.h"
#include "compresscoder.h"
#include "xmpcoder.h"
#include "loghelper.h"

const uint32_t FTD_TID_ReqUserLogin = 0x00001001;
const uint32_t FTD_TID_ReqUserLogout=0x00001003;

const uint32_t FTD_FID_Dissemination=0x0001;
const uint32_t FTD_FID_ReqUserLogin = 0x000A;
const uint32_t FTD_FID_ReqUserLogout=0x000C;

/*
///信息分发
    ///序列系列号   CSequenceSeriesType SequenceSeries;  UFWordType
    ///序列号       CSequenceNoType SequenceNo;    UFIntType
*/
/*
    ///交易日    CDateType   TradingDay;  DefineStringType(8,DateType)
    ///交易用户代码    CUserIDType UserID;      DefineStringType(15,UserIDType)
    ///会员代码    CParticipantIDType  ParticipantID; DefineStringType(10,ParticipantIDType)
    ///密码    CPasswordType   Password;   DefineStringType(40,PasswordType)
    ///用户端产品信息    CProductInfoType    UserProductInfo; DefineStringType(40,ProductInfoType)
    ///接口端产品信息    CProductInfoType    InterfaceProductInfo; DefineStringType(40,ProductInfoType)
    ///协议信息    CProtocolInfoType   ProtocolInfo; DefineStringType(40,ProtocolInfoType)
    ///数据中心代码    CDataCenterIDType   DataCenterID; DefineUFType(UFIntType,DataCenterIDType)
*/
static void CreateUserLoginFtpPacket(protocol::CFtdEncoder &encdoer) {
    protocol::CField field(FTD_FID_ReqUserLogin);

    field.AddValue("20120425", 8);
    field.AddValue("001002", 15);
    field.AddValue("0010", 10);
    field.AddValue("111111", 40);
    field.AddValue("UserProductInfo", 40);
    field.AddValue("InterfaceProductInfo", 40);
    field.AddValue("ProtocolInfo", 40);
    field.AddValue(1);

    protocol::CField disseminationField(FTD_FID_Dissemination);
    disseminationField.AddUint16(100);
    disseminationField.AddUint16(0);
    BS_XLOG(XLOG_DEBUG, "%s, sizeof(SFieldHeader)[%d], field_len[%d], field:\n%s\n", __FUNCTION__, sizeof(protocol::SFieldHeader), field.GetLen(),
        GetBinaryDumpInfo((const char *)(disseminationField.GetBuffer()), disseminationField.GetLen()).c_str());


    encdoer.SetStreamType(protocol::E_FTD_STREAM_DIALOG);
    encdoer.SetTid(FTD_TID_ReqUserLogin);
    encdoer.SetSeqNo(1);
    encdoer.SetSequestId(2);
    encdoer.AddField(field.GetBuffer(), field.GetLen());
    encdoer.AddField(disseminationField.GetBuffer(), disseminationField.GetLen());
}


/*
///交易用户代码   CUserIDType UserID;
///会员代码       CParticipantIDType  ParticipantID;
*/
static void CreateUserLogoutFtpPacket(protocol::CFtdEncoder &encdoer) {
    protocol::CField field(FTD_FID_ReqUserLogout);

    field.AddValue("001002", 15);
    field.AddValue("0010", 10);
    //BS_XLOG(XLOG_DEBUG, "%s, sizeof(SFieldHeader)[%d], field_len[%d], field:\n%s\n", __FUNCTION__, sizeof(protocol::SFieldHeader), field.GetLen(),
    //    GetBinaryDumpInfo((const char *)(field.GetBuffer()), field.GetLen()).c_str());

    encdoer.SetStreamType(protocol::E_FTD_STREAM_DIALOG);
    encdoer.SetTid(FTD_TID_ReqUserLogout);
    encdoer.SetSeqNo(2);
    encdoer.SetSequestId(3);
    encdoer.AddField(field.GetBuffer(), field.GetLen());
    encdoer.AddField(field.GetBuffer(), field.GetLen());
}

static void CreateXmpPacket(const protocol::CFtdEncoder &encoder, protocol::CXmpEncoder &xmp, protocol::ECompressMethod compress_method=protocol::E_COMPRESS_NONE) {
    BS_XLOG(XLOG_DEBUG, "%s, sizeof(SFtdHeader)[%d], ftd_len[%d], ftd:\n%s\n", __FUNCTION__, sizeof(protocol::SFtdHeader), encoder.GetLen(),
        GetBinaryDumpInfo((const char *)(encoder.GetBuffer()), encoder.GetLen()).c_str());

    protocol::CCompressEncoder compress(compress_method);
    //protocol::CCompressEncoder compress(protocol::E_COMPRESS_NONE);
    compress.SetBody(encoder.GetBuffer(), encoder.GetLen());
    BS_XLOG(XLOG_DEBUG, "%s, sizeof(SCompressHeader)[%d], compress_len[%d], ftd:\n%s\n", __FUNCTION__, sizeof(protocol::SCompressHeader), compress.GetLen(),
        GetBinaryDumpInfo((const char *)(compress.GetBuffer()), compress.GetLen()).c_str());

    xmp.SetXmpType(protocol::XMPTypeSCP);
    const char *ext_header = "extxmpheader";
    xmp.SetExHeader(protocol::XMPTagCompressMethod, ext_header, strlen(ext_header));
    xmp.SetBody(compress.GetBuffer(), compress.GetLen());
    //BS_XLOG(XLOG_DEBUG, "%s, xmp_len[%d], xmp:\n%s\n", __FUNCTION__, xmp.GetLen(), GetBinaryDumpInfo((const char *)(xmp.GetBuffer()), xmp.GetLen()).c_str());

/*
    protocol::CCompressDecoder compressdecoder;
    int len = compressdecoder.Decode(compress.GetBuffer(), compress.GetLen());
    BS_XLOG(XLOG_DEBUG, "%s, compressdecoder:\n%s\n", __FUNCTION__, GetBinaryDumpInfo((const char *)(compressdecoder.GetBody()), len).c_str());
    exit(-1);
*/
}
