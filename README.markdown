# STM32 NUCLEO-F302R8

This is a simple working template project for the NUCLEO-F302R8 development board.

## Features
* Toggles the user-LED and prints a message over the ST-LINK/V2-1 Virtual Com port, when the user-button is pressed.

## Building
First of all download submodules if not already done :

```sh
git submodule init
git submodule update
```

Then you need to build libopencm3:

```sh
cd libopencm3
make
cd ..
```

Finally you can build the app:

```sh
packager/packager.py
make
```

## Dependencies
* GNU Tools for ARM Embedded Processors: https://launchpad.net/gcc-arm-embedded
* Patched OpenOCD fork for uploading the firmware: https://github.com/cvra/OpenOCD
