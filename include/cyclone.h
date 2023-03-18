#ifndef CYCLONE_H_
#define CYCLONE_H_

#include <unistd.h>

#include <string>
#include <thread>

namespace cyclone {

struct Options {
  Options()
      : port(8080),
        websocket_ping_interval(0),
        num_threads(std::thread::hardware_concurrency()),
        root("."),
        access_control_allow_headers("*"),
        access_control_allow_methods("*"),
        access_control_allow_origin("*") {}
  size_t port;
  size_t websocket_ping_interval;
  size_t num_threads;
  std::string root;
  std::string access_control_allow_headers;
  std::string access_control_allow_methods;
  std::string access_control_allow_origin;
};

}  // namespace cyclone

#endif  // CYCLONE_H_
