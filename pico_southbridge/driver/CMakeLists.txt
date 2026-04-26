message(STATUS "building drivers...")

add_subdirectory(uart_log)
add_subdirectory(uart_bridge)
add_subdirectory(i2s_pcm)
add_subdirectory(btn_matrix)
add_subdirectory(joystick)
add_subdirectory(li_battery)
add_subdirectory(ir_remote)
add_subdirectory(system_time)
add_subdirectory(temp_builtin)
add_subdirectory(vibration_lra)

add_library(driver INTERFACE)

target_link_libraries(driver INTERFACE
  uart_log
  uart_bridge
  i2s_pcm
  btn_matrix
  joystick
  li_battery
  ir_remote
  system_time
  temp_builtin
  vibration_lra
)

message(STATUS "building drivers completed!")
