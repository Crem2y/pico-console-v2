message(STATUS "building drivers...")

add_subdirectory(uart_log)
add_subdirectory(uart_bridge)
add_subdirectory(i2s_pcm)
add_subdirectory(btn_matrix)
add_subdirectory(joystick)
add_subdirectory(adc_vsense)
add_subdirectory(bq25619)
add_subdirectory(mpu6050)
add_subdirectory(ir_rx)
add_subdirectory(ir_tx)
add_subdirectory(system_time)
add_subdirectory(temp_builtin)
add_subdirectory(temp_ntc)
add_subdirectory(vibration_lra)

add_library(driver INTERFACE)

target_link_libraries(driver INTERFACE
  uart_log
  uart_bridge
  i2s_pcm
  btn_matrix
  joystick
  adc_vsense
  bq25619
  mpu6050
  ir_rx
  ir_tx
  system_time
  temp_builtin
  temp_ntc
  vibration_lra
)

message(STATUS "building drivers completed!")
