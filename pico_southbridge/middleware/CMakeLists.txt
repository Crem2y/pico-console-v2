message(STATUS "building middlewares...")

add_subdirectory(bridge_protocol)
add_subdirectory(charger)
add_subdirectory(gamepad)
add_subdirectory(imu)
add_subdirectory(ir_link)
add_subdirectory(audio_system)
add_subdirectory(temperature)
add_subdirectory(vibration)


add_library(middleware INTERFACE)

target_link_libraries(middleware INTERFACE
  driver
  bridge_protocol
  charger
  gamepad
  imu
  ir_link
  audio_system
  temperature
  vibration
)

message(STATUS "building middlewares completed!")
