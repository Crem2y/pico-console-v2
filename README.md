# pico-console V2

A custom-built handheld console platform based on a multi-MCU architecture.

The system is built around two RP2350 microcontrollers:
one acting as the main processor and the other as a dedicated southbridge.

This project focuses on scalable embedded system design, including multi-processor coordination, custom protocols, and full-stack bare-metal development.

For the previous version of this project, see [pico-console](https://github.com/Crem2y/pico-console).

## Features

- Architecture: (planning)
  - Main CPU: RP2350A
  - Southbridge: RP2350B (handles input, peripherals, and auxiliary I/O)
  - Custom inter-MCU communication protocol

- Graphics:
  - 480x320 18-bit LCD (SPI protocol over HSTX)

- Audio:
  - Software mixing pipeline (multi-channel)
  - External DAC / amplifier support

- Input:
  - 16 buttons
  - 2 joysticks
  - Touchscreen
  - 6-axis IMU

Most input devices are handled via the southbridge,
while latency-sensitive components (e.g., touchscreen) can be connected directly to t

- Storage:
  - microSD

- Other:
  - Battery monitoring
  - IR communication (NEC)
  - Designed for extensibility (additional modules and peripherals)

This project focuses on exploring scalable embedded system design, including multi-processor coordination, custom protocols, and full-stack bare-metal development.

## How to build & upload firmware

1. Install CMake (at least version 3.13), Python 3, a native compiler, and a GCC cross compiler
```
sudo apt install cmake python3 build-essential gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```
2. Clone this repository and submodules:
```
$ git clone --recurse-submodules https://github.com/Crem2y/pico-console-v2.git
```
3. Launch the build script:
```
./pico_build.sh
```
4. Press the reset button twice to enter bootloader mode.

5. Upload the generated `.uf2` file to your board.

6. (Optional) Launch the clean script to remove build artifacts:
```
./pico_clean.sh
```

## Photos

- working!

## Schematics

- working!

---

## License

This project is licensed under the MIT License.  
See [LICENSE](./LICENSE) for details.

---

### Third-party components

See the `third_party_licenses/` directory for full details.