message(STATUS "building middlewares...")

add_subdirectory(gamepad)
add_subdirectory(imu)
add_subdirectory(ir_communication)
add_subdirectory(led_control)
# add_subdirectory(sound_system)
add_subdirectory(temperature)
add_subdirectory(ui_system)
add_subdirectory(vibration)


add_library(middleware INTERFACE)

target_link_libraries(middleware INTERFACE
  driver
  gamepad
#  imu
#  ir_communication
  led_control
#   sound_system
  temperature
#  ui_system
#  vibration
)

message(STATUS "building middlewares completed!")
