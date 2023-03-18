#ifndef CYCLONE_DEFINE_H_
#define CYCLONE_DEFINE_H_

#include <unistd.h>

#include <memory>
#include <string>

#include "civetweb/CivetServer.h"
#include "civetweb/civetweb.h"

namespace cyclone {

typedef struct mg_connection Connection;
typedef struct mg_request_info RequestInfo;
typedef CivetServer Application;
typedef std::function<void(Application*, Connection*)> Callback;

enum RequestMethod { GET = 0, POST, PUT, DELETE, PATCH };

}  // namespace cyclone

#endif  // CYCLONE_DEFINE_H_
