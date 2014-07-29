#ifndef _ZMT_BEACON_HTTPCOMMONTRANSFER_CJSON__H_
#define _ZMT_BEACON_HTTPCOMMONTRANSFER_CJSON__H_
#include <json.h>
#include <string>
#include <vector>
#include <algorithm>
#include <bson.h>

using std::string;
using std::vector;

class CJson2Bson {
 public:
    CJson2Bson();
    virtual ~CJson2Bson();
    virtual int Convert(const string &json, bson_t *bson);

 private:
    int ParseObject(struct json_object *obj, bson_t *bson);
    int Parse(const char *key, json_object *obj, bson_t *bson);
    int ParseArray(struct json_object *arrobject, bson_t *bson);

 private:
    json_object *root_;
};


#endif


