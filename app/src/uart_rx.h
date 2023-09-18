
#ifndef __UART_RX_H__
#define __UART_RX_H__

#include "pico/stdlib.h"

class UART_RX {
   public:
    typedef void (*UARTCallback)(const char *data);

    static UART_RX &getInstance(uint32_t buf_size);

    void putc(char c);
    char getc();
    void enable();
    void setUARTCallback(UARTCallback cb);
    void pollForData();

   private:
    static UART_RX *uart_rx;
    uint32_t rx_buf_size, rx_buf_mask;
    uart_inst *id;
    int irq;
    unsigned char *rx_buf;
    volatile unsigned char rx_head;
    volatile unsigned char rx_tail;
    volatile bool rx_has_data, awaiting_timer;
    UARTCallback callback;
    UART_RX(const UART_RX &) = delete;
    UART_RX &operator=(const UART_RX &) = delete;

    UART_RX(uint32_t rx_buf_size);
    ~UART_RX();

    static void interruptHandler();
    static int64_t timer_callback(alarm_id_t alarm_id, void *user_data);

    unsigned char receive_byte(void);
    unsigned char data_in_recieve_buffer(void);
    void interruptHandler_();
};
#endif