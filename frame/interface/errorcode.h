#ifndef APE_COMMON_ERROR_CODE_H_
#define APE_COMMON_ERROR_CODE_H_

namespace ape {
namespace common {

typedef enum {
  SUCCESS = 0,
  ERROR_TIME_OUT = -10260001,
  ERROR_PEER_CLOSE = -10260002, 
  ERROR_USER_CLOSE = -10260003, 
  ERROR_UNDEFINED = -10260100
}EErrorCode;

}
}

#endif
