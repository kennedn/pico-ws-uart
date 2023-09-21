cmake_minimum_required(VERSION 3.13)
file(READ static.html PICO_WS_SERVER_STATIC_HTML_HEX HEX)

project(pico-ws-uart C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_library(lwipopts_provider INTERFACE)
target_include_directories(lwipopts_provider INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_definitions(lwipopts_provider INTERFACE
  # CYW43_HOST_NAME=\"LG_TV\"
  PICO_WS_SERVER_STATIC_HTML_HEX=\"${PICO_WS_SERVER_STATIC_HTML_HEX}\"
  ENABLE_DEBUG=1      # pico-ws-server enable debug logging
  # NDEBUG=1          # LWIP disable debug logging
)

add_executable(pico-ws-uart
  src/pico-ws-uart.cpp
  src/uart_rx.cpp
)

target_link_libraries(pico-ws-uart
  pico_stdlib
  pico_cyw43_arch_lwip_poll
  pico_ws_server
)

target_compile_definitions(pico-ws-uart PRIVATE
  PICO_DEFAULT_UART_TX_PIN=4
  PICO_DEFAULT_UART_RX_PIN=5
  PICO_DEFAULT_UART=1
  PICO_DEFAULT_UART_BAUD_RATE=9600
  WIFI_SSID=\"${WIFI_SSID}\"
  WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
  MAX_WS_CONNECTIONS=2
  RX_BUFFER_SIZE=32           # Maximum UART RX message size
  CONTROL_CHAR=\'x\'          # When set, pollForData will wait for a control character before firing 
                              # When unset, pollForData will fire immediatly after all data is read from UART
  WIFI_STATUS_POLL_MS=10000   # How often to check wifi is still connected
  ENABLE_DEBUG=1              # pico-ws-uart enable debug logging
  )

pico_add_extra_outputs(pico-ws-uart)
