#include "gtest/gtest.h"
#include "watchdogthread.h"
#include "controller.h"
#include "loghelper.h"
#include "configmanager.h"
#include "mongothread.h"
#include "filehelper.h"

#ifdef Test_WATCHDOG_THREAD
using namespace ape::monitor;
/*
TEST(WatchDogThread, TestFile) {
    std::string dir = "./faileddata/";
    if (0 != MakeDirectory(dir.c_str())) {
        BS_XLOG(XLOG_ERROR,"ConfigManager::%s, mkdir dir error[%s]\n", __FUNCTION__, dir.c_str());
    }
}*/
TEST(WatchDogThread, Dump) {
    ConfigManager::GetInstance()->Init("");
    zamp::database::CMongoThreadGroup::GetInstance()->Start();
    zamp::database::CMongoThreadGroup::GetInstance()->OnConnect("mongodb://127.0.0.1");
    WatchDogThread::GetInstance()->Start();
    time_t now = time(NULL);
    for (int i = 0; i < 5; ++i) {
        char szcurveid[32] = {0};
        std::string projectid = "failed_projectid";
        std::vector<zamp::monitor::SCurveInfo> curves;
        curves.push_back(SCurveInfo(now - i * 10, "53993afa557965ce4a8b4567", "commandid", "1.1.1.1", i));
        WatchDogThread::GetInstance()->OnActionFail(projectid, curves);
        
        SErrorInfo info("53904aa2557965ca028b4567", "1.2.3.4", "test msg", 1001);
        WatchDogThread::GetInstance()->OnErrorFail(now, 1, info);
        sleep(1);
    }
    while(1) {
        sleep(1);
    }
}
#endif
