
#include "pico/stdlib.h"

class UART_RX {
private:

    static UART_RX *uart_rx;
    uint32_t rx_buf_size, rx_buf_mask;
    uart_inst *id;
    int irq;
    unsigned char *rx_buf;
    volatile unsigned char rx_head;
    volatile unsigned char rx_tail;
    volatile bool rx_has_data;
    

    UART_RX(uint32_t rx_buf_size): rx_buf_size(rx_buf_size) {
        id = uart_get_instance(PICO_DEFAULT_UART);
        rx_buf_mask = rx_buf_size - 1;
        // assert(rx_buf_size & rx_buf_mask == 0 && "RX buffer size is not a power of 2");
        rx_buf = (unsigned char *) std::malloc(rx_buf_size * sizeof(unsigned char));
        if (rx_buf == nullptr) {
          assert(1 == 0);
        }
        // Select correct interrupt for the UART we are using
        irq = id == uart0 ? UART0_IRQ : UART1_IRQ;
        rx_head = 0;
        rx_tail = 0;
        rx_has_data = false;
    }

    unsigned char receive_byte(void) {
        unsigned char tmptail;

        while (rx_head == rx_tail)
            ;                                               // Wait for incoming data

        tmptail = (rx_tail + 1) & rx_buf_mask;              // Calculate buffer index
        rx_tail = tmptail;                                  // Store new index
        return rx_buf[tmptail];                             // Reverse the order of the bits in the data byte before it returns data from the buffer.
    }

    // Check if there is data in the receive buffer.
    unsigned char data_in_recieve_buffer(void) {
        return (rx_head != rx_tail);                        // Return 0 (FALSE) if the receive buffer is empty.
    }

    void interruptHandler_() {
        rx_has_data = false;
        unsigned char tmphead;

        tmphead = (rx_head + 1) & rx_buf_mask;     // Calculate buffer index


        if (tmphead == rx_tail) {
            uart_getc(id);                                 // Circular buffer is full, Drop character
        } else {
            rx_head = tmphead;
            rx_buf[tmphead] = uart_getc(id);
        }

      if (!uart_is_readable(id)) {
        rx_has_data = true;
      }
    }


    ~UART_RX() {
      if (rx_buf != nullptr) {
        std::free(rx_buf);
      }
    }

    UART_RX(const UART_RX&) = delete;
    UART_RX& operator=(const UART_RX&) = delete;

public:
    // Static method to get the instance of the Singleton
    static UART_RX& getInstance(uint32_t buf_size = 32) {
        if (uart_rx == nullptr) {
          uart_rx = new UART_RX(buf_size);
        }
        return *uart_rx;
    }

    static void interruptHandler() {
      UART_RX::uart_rx->interruptHandler_();
    }

    int get_irq() {
      return irq;
    }

    void putc(char c) {
      uart_putc(id, c);
    }

    char getc() {
      return uart_getc(id);
    }

    void enable_irq() {
      uart_set_irq_enables(id, true, false);
    }

    void buffer_to_cstring(void (*callback)(const char *payload)) {
      if (rx_has_data) {
        rx_has_data = false;
        char tmp_buf[rx_buf_mask + 1];
        char *tmp_buf_ptr = tmp_buf;
        while(data_in_recieve_buffer()) {
            *(tmp_buf_ptr++) = receive_byte();
        }
        *tmp_buf_ptr = '\0';
        callback(tmp_buf);
      }
    }
};


