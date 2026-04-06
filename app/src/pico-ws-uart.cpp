#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico_ws_server/web_socket_server.h"
#include "hardware/watchdog.h"
#include "uart_rx.h"
#include "debug.h"

WebSocketServer server(MAX_WS_CONNECTIONS);
UART_RX* UART_RX::uart_rx = nullptr;
UART_RX& uart = UART_RX::getInstance(RX_BUFFER_SIZE);
uint32_t sw_timer;
// Updated when ws connections is not saturated
uint32_t ws_has_connections_ms = 0;


void on_connect(WebSocketServer& server, uint32_t conn_id) {
  DEBUG("WebSocket opened");
}

void on_disconnect(WebSocketServer& server, uint32_t conn_id) {
  DEBUG("WebSocket closed");
}

void on_message(WebSocketServer& server, uint32_t conn_id, const void* data, size_t len) {
  DEBUG("WebSocket message received: %.*s", (int)len, (const char*)data);
  for(size_t i=0; i < len; i++) {
    uart.putc(*((char*)data + i));
  }
}

void on_uart(const char *payload) {
  server.broadcastMessage(payload);
}

int main() {
  uint32_t now = to_ms_since_boot(get_absolute_time());
  sw_timer = now;
  ws_has_connections_ms = now;

  stdio_init_all();

  if (cyw43_arch_init() != 0) {
      DEBUG("cyw43_arch_init failed");
      while (1) tight_loop_contents();
  }

  cyw43_arch_enable_sta_mode();

  DEBUG("Connecting to Wi-Fi...");
  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_STATUS_POLL_MS)) {
      DEBUG("Failed to connect to Wi-Fi");
      watchdog_reboot(0, 0, 0);
      while (1) tight_loop_contents();
  }
  DEBUG("Connected.");

  server.setConnectCallback(on_connect);
  server.setCloseCallback(on_disconnect);
  server.setMessageCallback(on_message);

  bool server_ok = server.startListening(80);
  if (!server_ok) {
      DEBUG("Failed to start WebSocket server");
      while (1) tight_loop_contents();
  }
  DEBUG("WebSocket server started");

  uart.setUARTCallback(on_uart);
  uart.enable();
  
  DEBUG("UART interface started");
  watchdog_enable(WATCHDOG_MS, 1);
  while (1) {
      uint32_t now = to_ms_since_boot(get_absolute_time());

      if ((now - sw_timer) > WIFI_STATUS_POLL_MS) {
          sw_timer = now;

          DEBUG("status: ws_connection_count=%lu/%lu ws_age=%lu/%lu", 
              server.getConnectionCount(), 
              MAX_WS_CONNECTIONS,
              (unsigned long)(now - ws_has_connections_ms), 
              WATCHDOG_MS);

          if (cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) <= 0) {
              DEBUG("Wi-Fi link lost, rebooting");
              watchdog_reboot(0, 0, 0);
              while (1) tight_loop_contents();
          }
      }

      cyw43_arch_poll();
      uart.pollForData();

      // Track the last time we had available connections
      if (server.getConnectionCount() < MAX_WS_CONNECTIONS) {
          ws_has_connections_ms = now;
      }

      // Only pet the watchdog if websocket connection count has not been pinned at MAX_WS_CONNECTIONS for too long.
      bool ws_not_stuck_full = (now - ws_has_connections_ms) < WATCHDOG_MS;
      if (ws_not_stuck_full) {
          watchdog_update();
      }
  }
}
