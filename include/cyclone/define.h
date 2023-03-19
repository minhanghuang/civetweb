#ifndef CYCLONE_DEFINE_H_
#define CYCLONE_DEFINE_H_

#include <map>
#include <string>
#include <thread>
#include <unordered_map>

#include "cyclone/civetweb/CivetServer.h"

#define CYCLONE_HEADER_CONTENT_TYPE "application/json; charset=utf-8"

namespace cyclone {

typedef mg_connection Connection;
typedef mg_request_info RequestInfo;
typedef CivetServer Server;
typedef std::function<void(Server*, Connection*)> Callback;
typedef std::string URL;
typedef unsigned char OpCode;

enum class RequestMethod {
  GET = 0,
  POST,
  PUT,
  DELETE,
  PATCH,
};

enum WebSocketOpCode {
  CONTINUATION = MG_WEBSOCKET_OPCODE_CONTINUATION,
  TEXT = MG_WEBSOCKET_OPCODE_TEXT,
  BINARY = MG_WEBSOCKET_OPCODE_BINARY,
  CONNECTION_CLOSE = MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE,
  PING = MG_WEBSOCKET_OPCODE_PING,
  PONG = MG_WEBSOCKET_OPCODE_PONG,
};

}  // namespace cyclone

#endif  // CYCLONE_DEFINE_H_
