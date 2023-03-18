#ifndef CYCLONE_DEFINE_H_
#define CYCLONE_DEFINE_H_

#include <map>
#include <string>
#include <thread>
#include <unordered_map>

#include "cyclone/civetweb/CivetServer.h"

namespace cyclone {

typedef mg_connection Connection;
typedef mg_request_info RequestInfo;
typedef CivetServer Server;
typedef std::function<void(Server*, Connection*)> Callback;
typedef std::string URL;

enum class RequestMethod {
  GET = 0,
  POST,
  PUT,
  DELETE,
  PATCH,
};

}  // namespace cyclone

#endif  // CYCLONE_DEFINE_H_
