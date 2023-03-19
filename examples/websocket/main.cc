#include <iostream>
#include <memory>

#include "cyclone/cyclone.h"
#include "cyclone/define.h"
#include "cyclone/options.h"
#include "cyclone/web.h"

class MyWebSocketApi : public cyclone::websocket::WebSocketHandler {
 public:
  void Open(cyclone::Server* server, const cyclone::Connection* conn) override {
    std::cout << "open" << std::endl;
  }

  void OnClose(cyclone::Server* server,
               const cyclone::Connection* conn) override {
    std::cout << "close" << std::endl;
  }

  void OnPong(cyclone::Server* server, cyclone::Connection* conn) override {
    std::cout << "pong" << std::endl;
    BroadcastData("ok", true);
  }

  void OnMessage(cyclone::Server* server, cyclone::Connection* conn,
                 const std::string& msg, int op_code) override {
    std::cout << "message" << std::endl;
    if (op_code == cyclone::WebSocketOpCode::TEXT) {
      std::cout << "TEXT data: " << msg << std::endl;
    }
  }
};

int main(int argc, char* argv[]) {
  std::cout << "Cyclone Simple Examples." << std::endl;

  // options
  cyclone::Options options;
  options.port = 8090;
  options.num_threads = 2;
  std::cout << "options port:" << options.port << std::endl;

  // api
  MyWebSocketApi api;

  // app
  auto app = std::make_shared<cyclone::web::Application>();
  app->Init(options);
  app->AddHandler("/cyclone/examples/ws/", &api);
  app->Spin();

  return 0;
}
