message(STATUS "building middlewares...")

# add_subdirectory(gamepad)
# add_subdirectory(imu)
# add_subdirectory(ir_communication)
add_subdirectory(audio_system)
add_subdirectory(temperature)
# add_subdirectory(vibration)


add_library(middleware INTERFACE)

target_link_libraries(middleware INTERFACE
  driver
#  gamepad
#  imu
#  ir_communication
  audio_system
  temperature
#  vibration
)

message(STATUS "building middlewares completed!")
