#include "loghelper.h"
#include "controller.h"
#include "mongoclient.h"
#include "cjson.h"
#include <gtest/gtest.h>

#ifdef Test_Yali_MultiThread

using namespace ape::database;

const int yalicount = 100000;
const char *collection = "zampda_v2.material_new";

typedef struct stThreadPara {
    CMongoClient *client;
    bson_t *bson;
}SThreadPara;

void *run(void *params) {
    SThreadPara *para = (SThreadPara *)params;
    for(int i = 0; i < yalicount; ++i) {
        std::vector<bson_t *> result;
        para->client->Query(&result, collection, para->bson, NULL, 0, 10);
        for (std::vector<bson_t *>::iterator itr = result.begin(); itr != result.end(); ++itr) {
            char *str = bson_as_json (*itr, NULL);
            //BS_XLOG(XLOG_DEBUG, "%s\n", str);
            bson_free(str);
            bson_destroy(*itr);
        }
    }
    
}

void YaliInMultiThread(int threadnum, const char *json) {
    CJson2Bson convert;
    bson_t *bson = bson_new();
    if (0 != convert.Convert(json, bson)) {
        bson_destroy(bson);
        BS_XLOG(XLOG_WARNING, "illegal json: %s\n", json);
        return;
    }
    
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * threadnum);
    SThreadPara *params = (SThreadPara *)malloc(sizeof(SThreadPara) * threadnum);
    for (int i = 0; i < threadnum; ++i) {
        params[i].bson = bson;
        params[i].client = new CMongoClient();
        params[i].client->Connect("mongodb://127.0.0.1/");
    }
    
    struct timeval before, end;
    struct timezone tz;
    gettimeofday(&before, &tz);
    int count = yalicount * threadnum;
    BS_XLOG(XLOG_FATAL,"test[%s]\n",json);
    BS_XLOG(XLOG_FATAL,"start, count[%d] time[%u,%u]\n",count,before.tv_sec,before.tv_usec);
    
    for (int i = 0; i < threadnum; ++i) {
        if((pthread_create(&(threads[i]),NULL,run,(void*)(&(params[i]))))<0) {
            BS_XLOG(XLOG_ERROR, "create pthread[%d] error\n",i);
            exit(-2);
        }
    }
    
    for (int i = 0; i < threadnum; ++i) {
        int err = pthread_join(threads[i], NULL);
        if (err != 0) {
            BS_XLOG(XLOG_ERROR, "pthread_join[%d] error\n",i);
        }
    }
    
    gettimeofday(&end, &tz);
    float interval = (end.tv_sec - before.tv_sec) + (float)(end.tv_usec - before.tv_usec)/1000000;
    BS_XLOG(XLOG_FATAL,"after, time[%u,%u], req/s[%.3f]\n\n",end.tv_sec,end.tv_usec,(float)count/interval);
    
    for (int i = 0; i < threadnum; ++i) {
        delete params[i].client;
    }
    bson_destroy(bson);    
    free(threads);    
    free(params);
}
TEST(CMongoClient, Yali_MultiThread) {
    int threadnum = 7;
    YaliInMultiThread(threadnum, "{\"company_id\":8,\"product_id\":\"217381478\"}");
    YaliInMultiThread(threadnum, "{ \"company_id\" : 75 , \"product_id\" : { \"$in\" : [ \"100-506651\" , \"100-205209\" , \"100-209670\" , \"100-550507\" , \"100-506652\"]}}");
    YaliInMultiThread(threadnum, "{ \"company_id\" : 8 , \"args.arg_4.code\" : \"306488\"}");
}
 
   

#endif


