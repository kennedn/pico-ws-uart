#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico_ws_server/web_socket_server.h"
#include "uart_rx.h"

WebSocketServer server(2);
UART_RX& uart = UART_RX::getInstance();

extern "C" void UART_ISR() {
    uart.interruptHandler();
}

void on_connect(WebSocketServer& server, uint32_t conn_id) {
  printf("WebSocket opened\n");
  server.sendMessage(conn_id, "hello");
}

void on_disconnect(WebSocketServer& server, uint32_t conn_id) {
  printf("WebSocket closed\n");
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
  stdio_init_all();

  if (cyw43_arch_init() != 0) {
    printf("cyw43_arch_init failed\n");
    while (1) tight_loop_contents();
  }

  cyw43_arch_enable_sta_mode();

  printf("Connecting to Wi-Fi...\n");
  do {} while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000));
  printf("Connected.\n");


  server.setConnectCallback(on_connect);
  server.setCloseCallback(on_disconnect);
  server.setMessageCallback(on_message);

  bool server_ok = server.startListening(80);
  if (!server_ok) {
    printf("Failed to start WebSocket server\n");
    while (1) tight_loop_contents();
  }
  printf("WebSocket server started\n");

  

  // // And set up and enable the interrupt handlers
  irq_set_exclusive_handler(uart.get_irq(), UART_ISR);
  irq_set_enabled(uart.get_irq(), true);

  uart.enable_irq();
  
  printf("UART interface started\n");

  while (1) {
    cyw43_arch_poll();
    uart.buffer_to_callback(on_uart);
  }
}
