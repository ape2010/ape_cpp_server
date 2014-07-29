#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"
#include "loghelper.h"
#include "timermanager.h"
#include "threadtimer.h"
#include "events.h"
#include "testthread.h"


using namespace ape::common;
using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;

void clean() {
    BS_XLOG(XLOG_DEBUG," ========== %s ========\n", __FUNCTION__);
    TestThreadGroup::Release();
    XLOG_DESTROY();
}

int main(int argc, char **argv) {
    XLOG_INIT("log.properties", true);
    XLOG_REGISTER(BUSINESS_MODULE, "business");
    BS_XLOG(XLOG_DEBUG," ========== %s ========\n", __FUNCTION__);
    
    
    atexit(clean);
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    
}
