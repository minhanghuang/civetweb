#include <iostream>
#include <memory>

#include "cyclone/cyclone.h"
#include "cyclone/options.h"
#include "cyclone/web.h"

class MyApi1 : public cyclone::web::RequestHandler {
 public:
  void Get() override {
    std::cout << "simple get" << std::endl;
    Response("simple get");
  }

  void Post() override {
    std::cout << "simple post" << std::endl;
    Response("simple get");
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
  auto api1 = std::make_shared<MyApi1>();

  // app
  auto app = std::make_shared<cyclone::web::Application>();
  app->Init(options);
  app->AddHandler("/cyclone/examples/simple/", api1);
  app->Spin();

  return 0;
}
