#include "gtest/gtest.h"
#include "httpparser.h"
#include "controller.h"
#include "loghelper.h"

#ifdef Test_HttpResponseEncoder
using namespace ape::protocol;
using namespace ape::message;
using namespace ape::common;

TEST(CHttpParser, Encode1) {
    CHttpParser encoder;
    SHttpMessage message;
    message.code = 200;
    message.keepalive = true;
    message.httpversion = SHttpMessage::HTTP_1_1;
    message.body = "testbody";
    
    CBuffer response;
    EXPECT_EQ(0, encoder.Encode(&message, &response));
    message.Dump();
    BS_XLOG(XLOG_DEBUG,"%s response:\n%s\n",__FUNCTION__, std::string(response.base(), response.len()).c_str());
}
TEST(CHttpParser, Encode2) {
    CHttpParser encoder;
    SHttpMessage message;
    message.code = 200;
    message.keepalive = false;
    message.httpversion = SHttpMessage::HTTP_1_0;
    message.body = "testbody";
    
    message.AddHeader("Content-Encoding", "gzip");
    message.AddHeader("Content-Type", "Content-TypeContent-TypeContent-Type");
    
    message.SetCookie("BDSVRTM", "aaaaaaa", "/", "", time(NULL) + 3600);
    message.SetCookie("H_PS_PSSID", "6225_6549_6249_1439_5225", "/", ".baidu.com");
    
    CBuffer response;
    EXPECT_EQ(0, encoder.Encode(&message, &response));
    message.Dump();
    BS_XLOG(XLOG_DEBUG,"%s response:\n%s\n",__FUNCTION__, std::string(response.base(), response.len()).c_str());
    
}

#endif
