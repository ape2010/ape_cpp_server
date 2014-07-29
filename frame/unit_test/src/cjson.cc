#include "cjson.h"
#include <algorithm>
#include <cstring>
#include "loghelper.h"

CJson2Bson::CJson2Bson() : root_(NULL) {
}
CJson2Bson::~CJson2Bson() {
  if (root_) {
    json_object_put(root_);
    root_ = NULL;
  }
}
int CJson2Bson::Convert(const string &json, bson_t *bson) {
    if (root_) {
        json_object_put(root_);
        root_ = NULL;
    }
    root_ = json_tokener_parse(json.c_str());
    if(is_error(root_)) {
        BS_XLOG(XLOG_DEBUG, "CJson2Bson::%s illegal json string[%s]\n", __FUNCTION__, json.c_str());
        return -1;
    }
  
    if (json_type_object != json_object_get_type(root_)) {
        BS_XLOG(XLOG_DEBUG, "CJson2Bson::%s illegal json string[%s], not a json object\n", __FUNCTION__, json.c_str());
        json_object_put(root_);
        return -2;
    }
    ParseObject(root_, bson);
    return 0;
}

int CJson2Bson::ParseObject(struct json_object *obj, bson_t *bson) {
    json_object_object_foreach(obj, strkey, val_obj) {
        Parse(strkey, val_obj, bson);
    }
  return 0;
}
int CJson2Bson::Parse(const char *key, json_object *obj, bson_t *bson) {
    if(obj == NULL) {
        return 0;
    }
  
    json_type type = json_object_get_type(obj);
    switch(type){
      case json_type_boolean : {
        BSON_APPEND_BOOL(bson, key, json_object_get_boolean(obj));
        break;
      }
      case json_type_double : {
        BSON_APPEND_DOUBLE(bson, key, json_object_get_double(obj));
        break;
      }
      case json_type_int : {
        BSON_APPEND_INT32(bson, key, json_object_get_int(obj));
        break;
      }
      case json_type_object : {
        bson_t child;
        BSON_APPEND_DOCUMENT_BEGIN(bson, key, &child);
        ParseObject(obj, &child);
        bson_append_document_end(bson, &child);
        break;
      }
      case json_type_array : {
        bson_t child;
        BSON_APPEND_ARRAY_BEGIN(bson, key, &child);
        ParseArray(obj, &child);
        bson_append_array_end(bson, &child);
        break;
      }
      case json_type_string : {
        BSON_APPEND_UTF8(bson, key, json_object_get_string(obj));
        break;
      }
      default: {
        break;
      }
    }

    return 0;
}
int CJson2Bson::ParseArray(struct json_object *arrobject, bson_t *bson) {
    struct json_object *obj;
    int arrlen = json_object_array_length(arrobject);
    for (int i = 0 ; i < arrlen; i++) {
        obj = json_object_array_get_idx(arrobject, i);
        json_type type = json_object_get_type(obj);
        switch(type){
          case json_type_boolean : {
            BSON_APPEND_BOOL(bson, "", json_object_get_boolean(obj));
            break;
          }
          case json_type_double : {
            BSON_APPEND_DOUBLE(bson, "", json_object_get_double(obj));
            break;
          }
          case json_type_int : {
            BSON_APPEND_INT32(bson, "", json_object_get_int(obj));
            break;
          }
          case json_type_object : {
            bson_t child;
            BSON_APPEND_DOCUMENT_BEGIN(bson, "", &child);
            ParseObject(obj, &child);
            bson_append_document_end(bson, &child);
            break;
          }
          case json_type_array : {
            bson_t child;
            BSON_APPEND_ARRAY_BEGIN(bson, "", &child);
            ParseArray(obj, &child);
            bson_append_array_end(bson, &child);
            break;
          }
          case json_type_string : {
            BSON_APPEND_UTF8(bson, "", json_object_get_string(obj));
            break;
          }
          default: {
            break;
          }
        }
    }

    return 0;
}

