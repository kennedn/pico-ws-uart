#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico_ws_server/web_socket_server.h"
#include "uart_rx.h"
#include "debug.h"

WebSocketServer server(MAX_WS_CONNECTIONS);
UART_RX* UART_RX::uart_rx = nullptr;
UART_RX& uart = UART_RX::getInstance(RX_BUFFER_SIZE);
uint32_t sw_timer;

void on_connect(WebSocketServer& server, uint32_t conn_id) {
  DEBUG("WebSocket opened");
}

void on_disconnect(WebSocketServer& server, uint32_t conn_id) {
  DEBUG("WebSocket closed");
}

void on_message(WebSocketServer& server, uint32_t conn_id, const void* data, size_t len) {
  for(size_t i=0; i < len; i++) {
    uart.putc(*((char*)data + i));
  }
}

void on_uart(const char *payload) {
  server.sendMessageAll(payload);
}

int main() {
  sw_timer = 0;

  stdio_init_all();

  if (cyw43_arch_init() != 0) {
    DEBUG("cyw43_arch_init failed");
    while (1) tight_loop_contents();
  }

  cyw43_arch_enable_sta_mode();

  DEBUG("Connecting to Wi-Fi...");
  do {} while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000));
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

  while (1) {
    // Check WiFI status periodically
    if(cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) < 0 && (to_ms_since_boot(get_absolute_time()) - sw_timer) > WIFI_STATUS_POLL_MS) {
      cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
      sw_timer = to_ms_since_boot(get_absolute_time());
    }
    cyw43_arch_poll();
    // Check for UART data periodically, this will fire the UARTCallback when data exists
    uart.pollForData();
  }
}
