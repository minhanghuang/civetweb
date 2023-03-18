#ifndef CYCLONE_WEB_H_
#define CYCLONE_WEB_H_

#include <unistd.h>

#include <string>

#include "civetweb/CivetServer.h"
#include "civetweb/civetweb.h"
#include "define.h"

namespace cyclone {
namespace web {

class RequestHandler : public CivetHandler {
 public:
  virtual void Get(Application* app, Connection* conn);
  virtual void Post(Application* app, Connection* conn);
  virtual void Put(Application* app, Connection* conn);
  virtual void Delete(Application* app, Connection* conn);
  virtual void Patch(Application* app, Connection* conn);

 private:
  bool handleGet(Application* app, Connection* conn) override;
  bool handlePost(Application* app, Connection* conn) override;
  bool handlePut(Application* app, Connection* conn) override;
  bool handleDelete(Application* app, Connection* conn) override;
  bool handlePatch(Application* app, Connection* conn) override;
  std::map<RequestMethod, Callback> callback_;
};

}  // namespace web
}  // namespace cyclone

#endif  // CYCLONE_WEB_H_
