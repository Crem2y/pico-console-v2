# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

if(PICO_SDK_PATH)
else()
    set(PICO_SDK_PATH ../../pico-sdk)
endif()
include(pico_sdk_import.cmake)

project(pico_console_bootloader C CXX ASM)

set(SW_VERSION_MAJOR 1)
set(SW_VERSION_MINOR 0)
set(SW_VERSION_PATCH 0)

# clock setup. if changed, you must (re)configure the cmake.
#add_compile_definitions(SYS_CLK_MHZ=200) # set clock to 200MHz (RP2040 maximum clock)
add_compile_definitions(SYS_CLK_MHZ=150) # set clock to 150MHz (RP2350 default clock)

pico_sdk_init()

add_executable(${PROJECT_NAME}
  src/hw_test_v2.cpp
  src/hw_config.c
)

add_subdirectory(common)
add_subdirectory(lib)
add_subdirectory(driver)
add_subdirectory(middleware)

target_include_directories(${PROJECT_NAME} PRIVATE
  ./include
)

target_link_libraries(${PROJECT_NAME}
  pico_stdlib
  pico_multicore
  pico_bootsel_via_double_reset # Double tap reset into bootrom
  common
  lib
  driver
  middleware
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  SW_VERSION_MAJOR=${SW_VERSION_MAJOR}
  SW_VERSION_MINOR=${SW_VERSION_MINOR}
  SW_VERSION_PATCH=${SW_VERSION_PATCH}
)

pico_set_program_version(
    ${PROJECT_NAME}
    "${SW_VERSION_MAJOR}.${SW_VERSION_MINOR}.${SW_VERSION_PATCH}"
)

pico_set_uf2_family(${PROJECT_NAME} "rp2350-arm-s")

pico_embed_pt_in_binary(${PROJECT_NAME}
  "${CMAKE_CURRENT_LIST_DIR}/partition.json"
)

target_link_options(${PROJECT_NAME} PRIVATE
  -Wl,--print-memory-usage
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

# set(CMAKE_SIZE arm-none-eabi-size)
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#   COMMAND ${CMAKE_SIZE} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf
#   COMMENT "RAM/Flash Usage : "
# )