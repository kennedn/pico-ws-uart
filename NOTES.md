Use a UART IRQ to trigger a server.sendMessage()
Can trigger the server.sendMessage() after a grace period of no more data or perhaps after a control character like `\n`

https://github.com/raspberrypi/pico-examples/blob/master/uart/uart_advanced/uart_advanced.c