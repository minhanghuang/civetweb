#ifndef CYCLONE_OPTIONS_H_
#define CYCLONE_OPTIONS_H_

#include <string>
#include <thread>

namespace cyclone {

struct Options {
  Options()
      : port(8080),
        websocket_ping_interval(0),
        num_threads(std::thread::hardware_concurrency()),
        root(""),
        access_control_allow_headers("*"),
        access_control_allow_methods("*"),
        access_control_allow_origin("*"),
        request_timeout_ms(200),
        enable_websocket_ping_pong(false) {}
  size_t port;
  size_t websocket_ping_interval;
  size_t num_threads;
  std::string root;
  std::string access_control_allow_headers;
  std::string access_control_allow_methods;
  std::string access_control_allow_origin;
  int request_timeout_ms;
  bool enable_websocket_ping_pong;
};

}  // namespace cyclone

#endif  // CYCLONE_OPTIONS_H_
