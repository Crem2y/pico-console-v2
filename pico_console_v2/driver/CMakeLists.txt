message(STATUS "building drivers...")

add_subdirectory(uart_log)
add_subdirectory(uart_bridge)
add_subdirectory(led_builtin)
add_subdirectory(led_status)
add_subdirectory(li_battery)
add_subdirectory(ili9488_40_hstx)
add_subdirectory(system_time)
add_subdirectory(temp_builtin)
add_subdirectory(xpt2046)

add_library(driver INTERFACE)

target_link_libraries(driver INTERFACE
  uart_log
  uart_bridge
  led_builtin
  led_status
  li_battery
  ili9488_40_hstx
  system_time
  temp_builtin
  xpt2046
)

message(STATUS "building drivers completed!")
