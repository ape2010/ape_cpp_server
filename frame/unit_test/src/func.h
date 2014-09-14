#ifndef _T_FUNC_H_
#define _T_FUNC_H_

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "loghelper.h"
#include <sys/time.h>

template <typename Func>
void ExecuteYali(Func func, const char *tips, int count = 100000) {
    boost::function<void ()> do_func = boost::function<void ()>(func);
    struct timeval before, end;
    struct timezone tz;
    gettimeofday(&before, &tz);
    BS_XLOG(XLOG_FATAL,"start[%s], count[%d] time[%u,%u]\n",tips,count,before.tv_sec,before.tv_usec);
    
    for(int i = 0; i < count; ++i) {
        do_func();
    }
    
    gettimeofday(&end, &tz);
    float interval = (end.tv_sec - before.tv_sec) + (float)(end.tv_usec - before.tv_usec)/1000000;
    BS_XLOG(XLOG_FATAL,"end[%s], time[%u,%u], req/s[%.3f]\n\n",tips,end.tv_sec,end.tv_usec,(float)count/interval);
}
#endif
