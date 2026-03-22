message(STATUS "building drivers...")

add_subdirectory(uart_log)
add_subdirectory(uart_bridge)
add_subdirectory(i2s_pcm)
add_subdirectory(li_battery)
add_subdirectory(ir_remote)
add_subdirectory(system_time)
add_subdirectory(temp_builtin)

add_library(driver INTERFACE)

target_link_libraries(driver INTERFACE
  uart_log
  uart_bridge
  i2s_pcm
  li_battery
  ir_remote
  system_time
  temp_builtin
)

message(STATUS "building drivers completed!")
