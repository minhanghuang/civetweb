#include <iostream>
#include <memory>

#include "cyclone/cyclone.h"
#include "cyclone/define.h"
#include "cyclone/options.h"
#include "cyclone/web.h"
#include "hello.pb.h"

class MyApi1 : public cyclone::web::RequestHandler {
 public:
  void Get(cyclone::Server* server, cyclone::Connection* conn) override {
    std::cout << "simple get" << std::endl;
    Point point;
    point.set_x(1.1);
    point.set_y(2.2);
    point.set_z(3.4);
    std::string response;
    point.SerializeToString(&response);
    Response(server, conn, response);
  }

  void Post(cyclone::Server* server, cyclone::Connection* conn) override {
    std::cout << "simple post" << std::endl;
    auto data = GetRequestData(conn);
    Point point;
    point.ParseFromString(data);
    std::cout << "point x: " << point.x() << std::endl;
    std::cout << "point y: " << point.y() << std::endl;
    std::cout << "point z: " << point.z() << std::endl;
    Response(server, conn, "simple get");
  }
};

int main(int argc, char* argv[]) {
  std::cout << "Cyclone Simple Protobuf Examples." << std::endl;

  // options
  cyclone::Options options;
  options.port = 8090;
  options.num_threads = 2;
  std::cout << "options port:" << options.port << std::endl;

  // api
  auto api1 = std::make_shared<MyApi1>();

  // app
  auto app = std::make_shared<cyclone::web::Application>();
  app->Init(options);
  app->AddHandler("/cyclone/examples/simple-pb/", api1);
  app->Spin();

  return 0;
}
