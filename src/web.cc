#include "cyclone/web.h"

namespace cyclone {
namespace web {

bool RequestHandler::handleGet(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::GET)) {
    this->Get(server, conn);
  } else {
    register_cb_[RequestMethod::GET](server, conn);
  }
  return true;
}

bool RequestHandler::handlePost(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::POST)) {
    this->Post(server, conn);
  } else {
    register_cb_[RequestMethod::POST](server, conn);
  }
  return true;
}

bool RequestHandler::handlePut(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::PUT)) {
    this->Put(server, conn);
  } else {
    register_cb_[RequestMethod::PUT](server, conn);
  }
  return true;
}

bool RequestHandler::handleDelete(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::DELETE)) {
    this->Delete(server, conn);
  } else {
    register_cb_[RequestMethod::DELETE](server, conn);
  }
  return true;
}

bool RequestHandler::handlePatch(Server* server, Connection* conn) {
  if (0 == register_cb_.count(RequestMethod::PATCH)) {
    this->Patch(server, conn);
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
                              const std::string& data) {
  mg_send_http_ok(conn, "serverlication/json; charset=utf-8", data.size());
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

int RequestHandler::AddResoposeHeader(Connection* conn,
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

Application::Application(const Options& options) : options_(options) {
  ParseParam();
}

Application::Application(Options* options) : options_(*options) {
  ParseParam();
}

void Application::ParseParam() {
  std::vector<std::string> options;
  options = {
      "document_root",
      options_.root,
      "listening_ports",
      std::to_string(options_.port),
      "access_control_allow_headers",
      options_.access_control_allow_headers,
      "access_control_allow_methods",
      options_.access_control_allow_methods,
      "access_control_allow_origin",
      options_.access_control_allow_origin,
  };
  server_ = std::make_shared<Server>(options);
}

void Application::Bind(const URL& url, RequestMethod method,
                       Callback callback) {
  std::lock_guard<std::mutex> guard(mutex_);
  RequestHandler::Ptr handler = nullptr;
  if (0 == handler_pool_.count(url)) {
    handler = std::make_shared<RequestHandler>();
    handler_pool_[url] = handler;
    AddHandle(url, handler);
  } else {
    handler = handler_pool_[url];
  }
  handler->RegisterMethod(method, callback);
}

void Application::Spin() {}

int Application::AddHandle(const URL& url, RequestHandler::Ptr handle) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (0 != handler_pool_.count(url)) {
    return -1;
  }
  handler_pool_[url] = handle;
  server_->addHandler(url, *handle);
  return 0;
}

int Application::AddHandle(const URL& url, RequestHandler* handle) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (0 != handler_pool_.count(url)) {
    return -1;
  }
  handler_pool_[url] = std::shared_ptr<RequestHandler>(handle);
  server_->addHandler(url, handle);
  return 0;
}

void Application::Get(const URL& url, Callback callback) {
  Bind(url, RequestMethod::GET, callback);
}

void Application::Post(const URL& url, Callback callback) {
  Bind(url, RequestMethod::POST, callback);
}

void Application::Put(const URL& url, Callback callback) {
  Bind(url, RequestMethod::PUT, callback);
}

void Application::Delete(const URL& url, Callback callback) {
  Bind(url, RequestMethod::DELETE, callback);
}

void Application::Patch(const URL& url, Callback callback) {
  Bind(url, RequestMethod::PATCH, callback);
}

}  // namespace web
}  // namespace cyclone
