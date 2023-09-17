#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico_ws_server/web_socket_server.h"

#define UART_ID uart_get_instance(PICO_DEFAULT_UART)

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define UART_RX_BUFFER_SIZE 32
#define UART_RX_BUFFER_MASK (UART_RX_BUFFER_SIZE - 1)
#if (UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK)
#error RX buffer size is not a power of 2
#endif

static unsigned char uart_rx_buffer[UART_RX_BUFFER_SIZE];
static volatile unsigned char uart_rx_head;
static volatile unsigned char uart_rx_tail;
static volatile bool uart_rx_has_data;

void uart_flush_rx_buffer(void) {
    uart_rx_head = 0;
    uart_rx_tail = 0;
    uart_rx_has_data = false;
}

// Returns a byte from the receive buffer. Waits if buffer is empty.
unsigned char uart_receive_byte(void) {
    unsigned char tmptail;

    while (uart_rx_head == uart_rx_tail)
        ;                                                   // Wait for incoming data

    tmptail = (uart_rx_tail + 1) & UART_RX_BUFFER_MASK;     // Calculate buffer index
    uart_rx_tail = tmptail;                                 // Store new index
    return uart_rx_buffer[tmptail];                         // Reverse the order of the bits in the data byte before it returns data from the buffer.
}

// Check if there is data in the receive buffer.
unsigned char uart_data_in_recieve_buffer(void) {
    return (uart_rx_head != uart_rx_tail);  // Return 0 (FALSE) if the receive buffer is empty.
}

// RX interrupt handler
void uart_rx_isr() {
    unsigned char tmphead;

    tmphead = (uart_rx_head + 1) & UART_RX_BUFFER_MASK;     // Calculate buffer index


    if (tmphead == uart_rx_tail) {
        uart_getc(UART_ID);                                 // Circular buffer is full, Drop character
    } else {
        uart_rx_head = tmphead;
        uart_rx_buffer[tmphead] = uart_getc(UART_ID);
    }


    if (!uart_is_readable(UART_ID)) {
        uart_rx_has_data = true;
    }
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
    uart_putc(UART_ID, *((char*)data + i));
  }
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

  WebSocketServer server(2);

  server.setConnectCallback(on_connect);
  server.setCloseCallback(on_disconnect);
  server.setMessageCallback(on_message);

  bool server_ok = server.startListening(80);
  if (!server_ok) {
    printf("Failed to start WebSocket server\n");
    while (1) tight_loop_contents();
  }
  printf("WebSocket server started\n");

  uart_flush_rx_buffer();

  // Select correct interrupt for the UART we are using
  int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

  // And set up and enable the interrupt handlers
  irq_set_exclusive_handler(UART_IRQ, uart_rx_isr);
  irq_set_enabled(UART_IRQ, true);

  // Now enable the UART to send interrupts - RX only
  uart_set_irq_enables(UART_ID, true, false);

  printf("UART interface started\n");

  while (1) {
    cyw43_arch_poll();
    if (uart_rx_has_data) {
      uart_rx_has_data = false;
      char tmp_buf[UART_RX_BUFFER_SIZE + 1];
      char *tmp_buf_ptr = tmp_buf;
      while(uart_data_in_recieve_buffer()) {
          *(tmp_buf_ptr++) = uart_receive_byte();
      }
      *tmp_buf_ptr = '\0';

      server.sendMessageAll(tmp_buf);
    }
  }
}
