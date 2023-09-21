#include "uart_rx.h"

#include <stdexcept>

#include "pico/stdlib.h"

// Static method to get the instance of the Singleton
UART_RX &UART_RX::getInstance(uint32_t buf_size = 32) {
    if (uart_rx == nullptr) {
        uart_rx = new UART_RX(buf_size);
    }
    return *uart_rx;
}

void UART_RX::interruptHandler() {
    UART_RX::uart_rx->interruptHandler_();
}

void UART_RX::putc(char c) {
    uart_putc(id, c);
}

char UART_RX::getc() {
    return uart_getc(id);
}

void UART_RX::enable() {
    uart_set_irq_enables(id, true, false);
}

void UART_RX::setUARTCallback(UARTCallback cb) { callback = cb; }

void UART_RX::pollForData() {
    if (rx_has_data) {
        rx_has_data = false;

        // Empty circular buffer and create a temporary cstring from the data
        char tmp_buf[rx_buf_size + 1];
        char *tmp_buf_ptr = tmp_buf;
        while (data_in_recieve_buffer()) {
            *(tmp_buf_ptr++) = receive_byte();
        }
        *tmp_buf_ptr = '\0';

        // Pass cstring to callback if configured
        if (callback) {
            callback(tmp_buf);
        }
    }
}

UART_RX::UART_RX(uint32_t rx_buf_size) : rx_buf_size(rx_buf_size) {
    id = uart_get_instance(PICO_DEFAULT_UART);
    rx_buf_mask = rx_buf_size - 1;
    // assert(rx_buf_size & rx_buf_mask == 0 && "RX buffer size is not a power of 2");
    if ((rx_buf_size & rx_buf_mask) != 0) {
        throw std::runtime_error("RX buffer size is not a power of 2");
    }
    rx_buf = (unsigned char *)std::malloc(rx_buf_size * sizeof(unsigned char));
    if (rx_buf == nullptr) {
        throw std::bad_alloc();
    }
    // Select correct interrupt for the UART we are using
    irq = id == uart0 ? UART0_IRQ : UART1_IRQ;
    rx_head = 0;
    rx_tail = 0;
    rx_has_data = false;
    awaiting_timer = false;
    callback = nullptr;
    // // And set up the interrupt handler
    irq_set_exclusive_handler(irq, interruptHandler);
    irq_set_enabled(irq, true);
}

UART_RX::~UART_RX() {
    if (rx_buf != nullptr) {
        std::free(rx_buf);
    }
}

unsigned char UART_RX::receive_byte(void) {
    unsigned char tmptail;

    while (rx_head == rx_tail)
        ;  // Wait for incoming data

    tmptail = (rx_tail + 1) & rx_buf_mask;  // Calculate buffer index
    rx_tail = tmptail;                      // Store new index
    return rx_buf[tmptail];                 // Reverse the order of the bits in the data byte before it returns data from the buffer.
}

// Check if there is data in the receive buffer.
unsigned char UART_RX::data_in_recieve_buffer(void) {
    return (rx_head != rx_tail);  // Return 0 (FALSE) if the receive buffer is empty.
}

void UART_RX::interruptHandler_() {
    rx_has_data = false;
    unsigned char tmphead;
    char c;

    tmphead = (rx_head + 1) & rx_buf_mask;  // Calculate buffer index
    c = uart_getc(id);
    if (tmphead != rx_tail) {
        rx_head = tmphead;
        rx_buf[tmphead] = c;
    }

#ifdef CONTROL_CHAR
    if (c == CONTROL_CHAR) {
#else
    if (!uart_is_readable(id)) {
#endif
        rx_has_data = true;
    }
}
