cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

file(READ static.html PICO_WS_SERVER_STATIC_HTML_HEX HEX)
# add_subdirectory(.. pico-ws-server)

project(pico-ws-uart C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_library(lwipopts_provider INTERFACE)
target_include_directories(lwipopts_provider INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(pico-ws-uart
  src/pico-ws-uart.cpp
  # src/uart_rx.h
)

target_link_libraries(pico-ws-uart
  pico_stdlib
  pico_cyw43_arch_lwip_poll
  pico_ws_server
)

target_compile_definitions(pico-ws-uart PRIVATE
  PICO_DEFAULT_UART_TX_PIN=0
  PICO_DEFAULT_UART_RX_PIN=1
  PICO_DEFAULT_UART=0
  PICO_DEFAULT_UART_BAUD_RATE=9600
  WIFI_SSID=\"${WIFI_SSID}\"
  WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
  DEBUG=1
  )

pico_add_extra_outputs(pico-ws-uart)
