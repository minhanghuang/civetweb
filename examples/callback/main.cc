#include <functional>
#include <iostream>
#include <memory>

#include "cyclone/cyclone.h"
#include "cyclone/define.h"
#include "cyclone/options.h"
#include "cyclone/web.h"

void MyGetApi(cyclone::Server* server, cyclone::Connection* conn) {
  std::cout << "callback get 2." << std::endl;
  cyclone::web::RequestHandler::Response(server, conn, "callback get 2.");
}

class MyApis {
 public:
  void InfoCallback3(cyclone::Server* server, cyclone::Connection* conn) {
    std::cout << "callback get 3." << std::endl;
    cyclone::web::RequestHandler::Response(server, conn, "callback get 3.");
  }
  void InfoCallback4(cyclone::Server* server, cyclone::Connection* conn) {
    std::cout << "callback get 4." << std::endl;
    cyclone::web::RequestHandler::Response(server, conn, "callback get 4.");
  }
};

int main(int argc, char* argv[]) {
  std::cout << "Cyclone Simple Examples." << std::endl;

  // options
  cyclone::Options options;
  options.port = 8090;
  options.num_threads = 2;
  std::cout << "options port:" << options.port << std::endl;

  // app
  auto app = std::make_shared<cyclone::web::Application>();
  app->Init(options);

  // 匿名函数
  app->Get("/cyclone/examples/callback/1/", [](cyclone::Server* server,
                                               cyclone::Connection* conn) {
    std::cout << "callback get" << std::endl;
    cyclone::web::RequestHandler::Response(server, conn, "callback get 1.");
  });

  // 回调函数
  app->Get("/cyclone/examples/callback/2/", MyGetApi);

  // std::function
  auto apis = MyApis();
  std::function<void(cyclone::Server*, cyclone::Connection*)> callback3 =
      std::bind(&MyApis::InfoCallback3, &apis, std::placeholders::_1,
                std::placeholders::_2);
  app->Get("/cyclone/examples/callback/3/", callback3);

  // class
  app->Get("/cyclone/examples/callback/4/",
           std::bind(&MyApis::InfoCallback4, apis, std::placeholders::_1,
                     std::placeholders::_2));

  app->Spin();

  return 0;
}
