# Build instructions

You can build the UF2 package to install on the Rasperberry Pi Pico using the instructions below, having first installed the SDK. Alternaticely the package is built automatically by a Github Action and made available as an Artefact.

1. mkdir build
2. cd build
3. export PICO_SDK_PATH=/Users/sfegan/RaspberryPi/pico/pico-sdk
4. cmake ..
5. cd flasher
6. make -j4
7. Hold button on PICO and connect USB power until device mounted in mass-storage mode
8. cp flasher.uf2 /Volumes/RPI-RP2
9. screen /dev/tty.usbmodem141101
