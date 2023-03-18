#ifndef __CLIENT_CONNECTION_H__
#define __CLIENT_CONNECTION_H__

#include <cstddef>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "http_handler.h"
#include "web_socket_handler.h"
#include "web_socket_message.h"

class WebSocketServerInternal;

class ClientConnection {
 public:
  ClientConnection(WebSocketServerInternal& server, struct tcp_pcb* pcb)
      : server(server), pcb(pcb), http_handler(*this), ws_handler(*this) {}

  bool process(struct pbuf* pb);
  bool sendRaw(const void* data, size_t size);

  void onClose();
  bool isClosing();

  void processWebSocketMessage(const void* payload, size_t size);
  bool sendWebSocketMessage(const char* payload);
  bool sendWebSocketMessage(const void* payload, size_t size);

 private:
  WebSocketServerInternal& server;
  struct tcp_pcb* pcb;
  HTTPHandler http_handler;
  WebSocketHandler ws_handler;
};

#endif
