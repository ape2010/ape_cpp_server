#include <sys/types.h>
#include <stdio.h>
#include <boost/algorithm/string.hpp> 
#include <sys/wait.h>
#include <vector>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <ape/loghelper.h>
#include <ape/netthreadholder.h>
#include <ape/xmlconfigparser.h>
#include "businesshandle.h"

#define __TESTAPE__VERSION__ "1.0.0"
#define DEBUG

void terminate(int signum) {
  usleep(500);
  _exit(0);
}

int run(int flag, int argc, char* argv[]) {
    XLOG_INIT("./conf/log.properties", true);
    XLOG_REGISTER(BUSINESS_MODULE, "business");

    if (flag != 0) {
        BS_XLOG(XLOG_FATAL,"--------reboot!!!--------\n");
    }
    const char *configfile = "./conf/config.xml";
    ape::common::CXmlConfigParser parser;
    if (0 != parser.ParseFile(configfile)) {
        BS_XLOG(XLOG_FATAL,"parse config[%s] failed, error[%s]\n", configfile, parser.GetErrorMessage().c_str());
        return -1;
    }

    sigset_t new_mask;
    sigfillset(&new_mask);
    sigset_t old_mask;
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
    
    std::vector<std::string> addrs = parser.GetParameters("server/addr");
    ape::net::NetHandleFactory<ape::testthrift::CBusinessHandle> factory;
    ape::net::CNetThreadHolder threadholder;
    threadholder.Start(&factory, parser.GetParameter("netthread", 3));
    if (0 != threadholder.StartServer(addrs)) {
        return -1;
    }

    pthread_sigmask(SIG_SETMASK, &old_mask, 0);
    signal(SIGTERM, terminate);

#if 1
    char c;
    while ( c = getchar()) {
        if(c=='q') {
            threadholder.Stop();
            XLOG_DESTROY();
            break;
        }
        else if(c=='p'){
            /** dump */
        } else if(c!='\n') {
            BS_XLOG(XLOG_WARNING,"can not regnize char [%c]\n",c);
        }
    }
#else
    while (1){
       sleep(1);
    }
#endif
}

bool process_arg(int argc, char* argv[]) {
    char c;
    while((c = getopt (argc,argv,"v")) != -1 ) {
        switch (c) {
          case 'v':
            printf("Server version: %s\n", __TESTAPE__VERSION__);
            printf("Build time: %s %s\n", __TIME__, __DATE__);
            return false;
          default:
            return true;
        }
    }
    return true;
}

int main(int argc, char* argv[])
{
  if(!process_arg(argc,argv))
    return 0;

#ifdef DEBUG
  run(0, argc, argv); 
#else
  if (0 == fork()) {
    run(0, argc, argv);
    exit(0);
  }

  struct timeval begin, end, interval;
  gettimeofday(&begin,0);
  while (1) {
    wait(NULL);
    gettimeofday(&end,0);
    if(end.tv_sec > begin.tv_sec) {
      sleep(1);
      if (0 == fork()) {
        run(-1, argc, argv);
        exit(0);
      }
    }
  }
#endif
  return 0;
}

