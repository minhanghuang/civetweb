#ifndef CYCLONE_WEB_H_
#define CYCLONE_WEB_H_

#include <memory>
#include <mutex>

#include "cyclone/civetweb/civetserver.h"
#include "cyclone/define.h"
#include "cyclone/options.h"

namespace cyclone {
namespace web {

class RequestHandler : public CivetHandler {
 public:
  typedef std::shared_ptr<RequestHandler> Ptr;
  virtual void Get(Server* server, Connection* conn);
  virtual void Post(Server* server, Connection* conn);
  virtual void Put(Server* server, Connection* conn);
  virtual void Delete(Server* server, Connection* conn);
  virtual void Patch(Server* server, Connection* conn);
  void Response(Server* server, Connection* conn, const std::string& data);
  int Write(Server* server, Connection* conn, const void* data, size_t len);
  const RequestInfo* GetRequestInfo(Connection* conn);
  std::string GetRequestData(Connection* conn);
  std::string GetParam(Connection* conn, const char* key,
                       size_t occurrence = 0);
  std::string GetCookie(Connection* conn, const std::string& name);
  // void SetCookie(Connection* conn, const std::string& value);
  std::string GetMethod(Connection* conn);
  int AddResoposeHeader(Connection* conn, const std::string& header,
                        const std::string& value);
  void RegisterMethod(RequestMethod method, Callback callback);

 private:
  bool handleGet(Server* server, Connection* conn) override;
  bool handlePost(Server* server, Connection* conn) override;
  bool handlePut(Server* server, Connection* conn) override;
  bool handleDelete(Server* server, Connection* conn) override;
  bool handlePatch(Server* server, Connection* conn) override;
  std::map<RequestMethod, Callback> register_cb_;
};

class Application {
 public:
  explicit Application(const Options& options);
  explicit Application(Options* options);
  void Spin();
  int AddHandle(const URL& url, RequestHandler::Ptr handle);
  int AddHandle(const URL& url, RequestHandler* handle);
  void Get(const URL& url, Callback callback);
  void Post(const URL& url, Callback callback);
  void Put(const URL& url, Callback callback);
  void Delete(const URL& url, Callback callback);
  void Patch(const URL& url, Callback callback);

 private:
  void ParseParam();
  void Bind(const URL& url, RequestMethod method, Callback callback);
  std::mutex mutex_;
  Options options_;
  std::map<std::string, RequestHandler::Ptr> handler_pool_;
  std::shared_ptr<Server> server_;
};

}  // namespace web
}  // namespace cyclone

#endif  // CYCLONE_WEB_H_
