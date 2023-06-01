#ifndef CYCLONE_WEBSOCKET_H_
#define CYCLONE_WEBSOCKET_H_

#include <iostream>
#include <memory>
#include <sstream>
#include <mutex>
#include <string>

#include "cyclone/civetweb/CivetServer.h"
#include "cyclone/define.h"
#include "cyclone/options.h"

namespace cyclone {
namespace websocket {

class WebSocketHandler : public CivetWebSocketHandler {
 public:
  typedef std::shared_ptr<WebSocketHandler> Ptr;
  thread_local static std::stringstream thread_data;
  thread_local static OpCode thread_opcode;
  ~WebSocketHandler() = default;
  WebSocketHandler() = default;
  virtual void Open(Server* server, const Connection* conn);
  virtual void OnMessage(Server* server, Connection* conn,
                         const std::string& msg, int op_code);
  virtual void OnPong(Server* server, Connection* conn);
  virtual void OnPing(Server* server, Connection* conn);
  virtual void OnClose(Server* server, const Connection* conn);
  virtual bool SendData(Connection* conn, const std::string& data,
                        bool skippable = true,
                        OpCode op_code = WebSocketOpCode::TEXT);
  virtual bool BroadcastData(const std::string& data, bool skippable,
                             OpCode op_code = WebSocketOpCode::TEXT);

 private:
  /**
   * @brief 客户端打算链接,还未完成链接, before handleReadyState()
   *
   * @param app
   * @param conn
   *
   * @return
   */
  bool handleConnection(Server* server, const Connection* conn) override;

  /**
   * @brief 客户端链接
   *
   * @param app
   * @param conn
   */
  void handleReadyState(Server* server, Connection* conn) override;
  bool handleData(Server* server, Connection* conn, int bits, char* data,
                  size_t data_len) override;
  void handleClose(Server* app, const Connection* conn) override;
  mutable std::mutex mutex_;
  std::map<Connection*, std::shared_ptr<std::mutex>> conns_;
};

}  // namespace websocket
}  // namespace cyclone

#endif  // CYCLONE_WEBSOCKET_H_
