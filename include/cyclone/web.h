#ifndef CYCLONE_WEB_H_
#define CYCLONE_WEB_H_

#include <unistd.h>

#include <memory>
#include <mutex>
#include <vector>

#include "cyclone/civetweb/CivetServer.h"
#include "cyclone/define.h"
#include "cyclone/options.h"
#include "cyclone/websocket.h"

namespace cyclone {
namespace web {

class RequestHandler : public CivetHandler {
 public:
  typedef std::shared_ptr<RequestHandler> Ptr;
  RequestHandler() : server_(nullptr), conn_(nullptr) {}
  virtual void Get();
  virtual void Post();
  virtual void Put();
  virtual void Delete();
  virtual void Patch();
  void Response(const std::string& data,
                const std::string& content_type = CYCLONE_HEADER_CONTENT_TYPE);
  static void Response(
      Server* server, Connection* conn, const std::string& data,
      const std::string& content_type = CYCLONE_HEADER_CONTENT_TYPE);
  int Write(const void* data, size_t len);
  static int Write(Server* server, Connection* conn, const void* data,
                   size_t len);
  const RequestInfo* GetRequestInfo();
  static const RequestInfo* GetRequestInfo(Connection* conn);
  std::string GetRequestData();
  static std::string GetRequestData(Connection* conn);
  std::string GetParam(const char* key, size_t occurrence = 0);
  static std::string GetParam(Connection* conn, const char* key,
                              size_t occurrence = 0);
  std::string GetCookie(const std::string& name);
  static std::string GetCookie(Connection* conn, const std::string& name);
  std::string GetMethod();
  static std::string GetMethod(Connection* conn);
  int AddResoposeHeader(const std::string& header, const std::string& value);
  static int AddResoposeHeader(Connection* conn, const std::string& header,
                               const std::string& value);
  void RegisterMethod(RequestMethod method, Callback callback);

 private:
  bool handleGet(Server* server, Connection* conn) override;
  bool handlePost(Server* server, Connection* conn) override;
  bool handlePut(Server* server, Connection* conn) override;
  bool handleDelete(Server* server, Connection* conn) override;
  bool handlePatch(Server* server, Connection* conn) override;
  Server* server_;
  Connection* conn_;
  std::map<RequestMethod, Callback> register_cb_;
};

class Application {
 public:
  ~Application();
  Application();
  void Init(const Options& options);
  void Init(Options* options);
  void Spin();
  void Stop();
  int AddHandler(const URL& url, RequestHandler::Ptr handler);
  int AddHandler(const URL& url, RequestHandler* handler);
  int AddHandler(const URL& url, websocket::WebSocketHandler::Ptr handler);
  int AddHandler(const URL& url, websocket::WebSocketHandler* handler);
  void Get(const URL& url, Callback callback);
  void Post(const URL& url, Callback callback);
  void Put(const URL& url, Callback callback);
  void Delete(const URL& url, Callback callback);
  void Patch(const URL& url, Callback callback);

 private:
  void CivetInit();
  void ParseParam(const Options& options);
  void BuildServer();
  void Bind(const URL& url, RequestMethod method, Callback callback);
  std::mutex handlers_mutex_;
  std::vector<std::string> options_;
  std::map<std::string, RequestHandler::Ptr> handlers_;
  std::map<std::string, websocket::WebSocketHandler::Ptr> ws_handlers_;
  std::shared_ptr<Server> server_;
};

}  // namespace web
}  // namespace cyclone

#endif  // CYCLONE_WEB_H_
