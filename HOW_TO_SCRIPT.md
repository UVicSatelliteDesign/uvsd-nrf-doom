# nRF-DOOM Build Guide

## Less-Automatable Dependencies

### Ubuntu

1. Install common dev tools
    ```bash
    sudo apt install make get wget minicom
    ```
1. Install the ARM Embedded Toolchain
    ```bash
    sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
    ```
1. Download the [nRF Command Line Tools](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download#infotabs).
    1. Install the bundled version of SEGGER JLink.
    2. Install the nRF Command Line Tools.

### MacOS

1. Install common dev tools
    ```bash
    brew install minicom
    ```
1. Install the ARM Embedded Toolchain
    ```bash
    brew install --cask gcc-arm-embedded
    ```
1. Download the [nRF Command Line Tools](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download#infotabs).
    1. Install the bundled version of SEGGER JLink.
    2. Install the nRF Command Line Tools.

## Set up Git repository

```bash
# download repo somewhere you keep your projects
git clone https://github.com/UVicSatelliteDesign/uvsd-nrf-doom.git ~/projects/uvsd-doom # or wherever you want your repo to be
cd ~/projects/uvsd-doom
```

## Run setup script

```bash
./setup.sh
```

The script will:

* Verify required development tools are installed
* Warn about missing recommended tools like `minicom` and J-Link
* Add your user to the `dialout` group on Linux for serial port access
* Install `nrfutil` and required Nordic plugins (if missing)
* Configure default `minicom` settings
* Download and install the Nordic nRF5 SDK v17.1.0 into the repository
* Apply compatibility fixes to the SDK source and driver configuration
* Detect your ARM Embedded GCC installation and update the SDK Makefile automatically
* Replace the bundled `nrfx` drivers with version `v2.4.0`

## Build

```bash
# network core first
cd nrfdoom_net
make

# then app core
cd ../nrfdoom/nrf5340dk/armgcc
make
```

*warnings about `_close`, `_fstat`, etc are normal.*

## Flash
The nRF5340 DK has two micro USB ports. Both must be connected for programming/flashing.

The debug port is placed midway along the short edge of the board. It should be connected to your PC for programming.

The nRF USB port is placed offset along the long edge of the board. It should be connected to a power source (which may or may not be your PC) to supply power to the board.

```bash
# check board is detected
nrfutil device list

# flash network first
cd nrfdoom_net
make flash

# then app
cd ../nrfdoom/nrf5340dk/armgcc
make flash
```

## Test

1. Check for connected devices
    ```bash
    nrfutil device list
    ```
    Make note of the output of this command. Two ports should be listed: depending on your platform they may look like `/dev/ttyACM1` or `/dev/tty.usbmodem0010500781223`.
2. Listen over the UART ports
    ```bash
    minicom -D <port> -b 115200
    ```
    You may need to test both ports and see which one works.

LED 3 should be on when running.

## SD card

```bash
# format as FAT32
sudo mkfs.vfat -F 32 /dev/sdX1

# get doom wad
wget https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad

# copy to card
sudo mount /dev/sdX1 /mnt
sudo cp doom1.wad /mnt/
sudo umount /mnt
```

---

**notes:**
- nrfdoom_net = network core (gamepad)
- nrfdoom = app core (actual game)
- network must be flashed before app
