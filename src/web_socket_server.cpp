#include "pico_ws_server/web_socket_server.h"

#include <memory>
#include <stdint.h>

#include "web_socket_server_internal.h"

WebSocketServer::WebSocketServer(uint32_t max_connections)
    : internal(std::make_unique<WebSocketServerInternal>(*this, max_connections)) {}
WebSocketServer::~WebSocketServer() {}

void WebSocketServer::setConnectCallback(ConnectCallback cb) {
  internal->setConnectCallback(cb);
}
void WebSocketServer::setMessageCallback(MessageCallback cb) {
  internal->setMessageCallback(cb);
}
void WebSocketServer::setCloseCallback(CloseCallback cb) {
  internal->setCloseCallback(cb);
}

bool WebSocketServer::startListening(uint16_t port) {
  return internal->startListening(port);
}

bool WebSocketServer::sendMessage(uint32_t conn_id, const char* payload) {
  return internal->sendMessage(conn_id, payload);
}
bool WebSocketServer::sendMessage(uint32_t conn_id, const void* payload, size_t payload_size) {
  return internal->sendMessage(conn_id, payload, payload_size);
}
