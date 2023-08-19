#include "cyclone/web.h"

#include <string>

namespace cyclone {
namespace web {

bool RequestHandler::handleGet(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::GET)) {
    Get(server, conn);
  } else {
    register_cb_[RequestMethod::GET](server, conn);
  }
  return true;
}

bool RequestHandler::handlePost(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::POST)) {
    Post(server, conn);
  } else {
    register_cb_[RequestMethod::POST](server, conn);
  }
  return true;
}

bool RequestHandler::handlePut(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::PUT)) {
    Put(server, conn);
  } else {
    register_cb_[RequestMethod::PUT](server, conn);
  }
  return true;
}

bool RequestHandler::handleDelete(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::DELETE)) {
    Delete(server, conn);
  } else {
    register_cb_[RequestMethod::DELETE](server, conn);
  }
  return true;
}

bool RequestHandler::handlePatch(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::PATCH)) {
    Patch(server, conn);
  } else {
    register_cb_[RequestMethod::PATCH](server, conn);
  }
  return true;
}

void RequestHandler::Get(Server* server, Connection* conn) {
  Response(server, conn, "get");
}

void RequestHandler::Post(Server* server, Connection* conn) {
  Response(server, conn, "post");
}

void RequestHandler::Put(Server* server, Connection* conn) {
  Response(server, conn, "put");
}

void RequestHandler::Delete(Server* server, Connection* conn) {
  Response(server, conn, "delete");
}

void RequestHandler::Patch(Server* server, Connection* conn) {
  Response(server, conn, "patch");
}

void RequestHandler::Response(Server* server, Connection* conn,
                              const std::string& data,
                              const std::string& content_type) {
  mg_send_http_ok(conn, content_type.c_str(), data.size());
  mg_write(conn, data.c_str(), data.size());
}

int RequestHandler::Write(Server* server, Connection* conn, const void* data,
                          size_t len) {
  return mg_write(conn, data, len);
}

std::string RequestHandler::GetRequestData(Connection* conn) {
  return Server::getPostData(conn);
}

std::string RequestHandler::GetParam(Connection* conn, const char* key,
                                     size_t occurrence) {
  std::string ret;
  Server::getParam(conn, key, ret, occurrence);
  return ret;
}

const RequestInfo* RequestHandler::GetRequestInfo(Connection* conn) {
  return mg_get_request_info(conn);
}

std::string RequestHandler::GetCookie(Connection* conn,
                                      const std::string& name) {
  std::string s;
  Server::getCookie(conn, name, s);
  return s;
}

std::string RequestHandler::GetMethod(Connection* conn) {
  return Server::getMethod(conn);
}

int RequestHandler::AddResponseHeader(Connection* conn,
                                      const std::string& header,
                                      const std::string& value) {
  if (header.empty() || value.empty()) {
    return -1;
  }
  mg_response_header_start(conn, 200);
  return mg_response_header_add(conn, header.c_str(), value.c_str(),
                                value.size());
}

void RequestHandler::RegisterMethod(RequestMethod method, Callback callback) {
  register_cb_[method] = callback;
}

Application::~Application() {}

Application::Application() {}

void Application::CivetInit() { mg_init_library(MG_FEATURES_DEFAULT); }

void Application::ParseParam(const Options& options) {
  options_.clear();
  options_ = {
      "document_root",
      options.root,
      "listening_ports",
      std::to_string(options.port),
      "access_control_allow_headers",
      options.access_control_allow_headers,
      "access_control_allow_methods",
      options.access_control_allow_methods,
      "access_control_allow_origin",
      options.access_control_allow_origin,
      "request_timeout_ms",
      std::to_string(options.request_timeout_ms),
      "enable_websocket_ping_pong",
      options.enable_websocket_ping_pong ? "yes" : "no",
  };
}

void Application::BuildServer() {
  server_ = std::make_shared<Server>(options_);
}

void Application::Bind(const URL& url, RequestMethod method,
                       Callback callback) {
  RequestHandler::Ptr handler = nullptr;
  if (0 == handlers_.count(url)) {
    handler = std::make_shared<RequestHandler>();
    handlers_[url] = handler;
    server_->addHandler(url, *handler);
  } else {
    handler = handlers_[url];
  }
  handler->RegisterMethod(method, callback);
}

void Application::Init(const Options& options) {
  CivetInit();
  ParseParam(options);
  BuildServer();
}

void Application::Init(Options* options) {
  CivetInit();
  ParseParam(*options);
  BuildServer();
}

void Application::Spin() {
  while (true) {
    sleep(1);
  }
}

void Application::Stop() {
  server_->close();
  mg_exit_library();
}

int Application::AddHandler(const URL& url, RequestHandler::Ptr handler) {
  std::lock_guard<std::mutex> guard(handlers_mutex_);
  if (0 != handlers_.count(url) || !handler) {
    return -1;
  }
  handlers_[url] = handler;
  server_->addHandler(url, *handler);
  return 0;
}

int Application::AddHandler(const URL& url, RequestHandler* handler) {
  std::lock_guard<std::mutex> guard(handlers_mutex_);
  if (0 != handlers_.count(url) || !handler) {
    return -1;
  }
  handlers_[url] = std::shared_ptr<RequestHandler>(handler);
  server_->addHandler(url, handler);
  return 0;
}

int Application::AddHandler(const URL& url,
                            websocket::WebSocketHandler::Ptr handler) {
  std::lock_guard<std::mutex> guard(handlers_mutex_);
  if (0 != ws_handlers_.count(url) || !handler) {
    return -1;
  }
  ws_handlers_[url] = handler;
  server_->addWebSocketHandler(url, *handler);
  return 0;
}

int Application::AddHandler(const URL& url,
                            websocket::WebSocketHandler* handler) {
  std::lock_guard<std::mutex> guard(handlers_mutex_);
  if (0 != ws_handlers_.count(url) || !handler) {
    return -1;
  }
  ws_handlers_[url] = std::shared_ptr<websocket::WebSocketHandler>(handler);
  server_->addWebSocketHandler(url, *handler);
  return 0;
}

void Application::Get(const URL& url, Callback callback) {
  std::lock_guard<std::mutex> guard(handlers_mutex_);
  Bind(url, RequestMethod::GET, callback);
}

void Application::Post(const URL& url, Callback callback) {
  std::lock_guard<std::mutex> guard(handlers_mutex_);
  Bind(url, RequestMethod::POST, callback);
}

void Application::Put(const URL& url, Callback callback) {
  std::lock_guard<std::mutex> guard(handlers_mutex_);
  Bind(url, RequestMethod::PUT, callback);
}

void Application::Delete(const URL& url, Callback callback) {
  std::lock_guard<std::mutex> guard(handlers_mutex_);
  Bind(url, RequestMethod::DELETE, callback);
}

void Application::Patch(const URL& url, Callback callback) {
  std::lock_guard<std::mutex> guard(handlers_mutex_);
  Bind(url, RequestMethod::PATCH, callback);
}

}  // namespace web
}  // namespace cyclone
