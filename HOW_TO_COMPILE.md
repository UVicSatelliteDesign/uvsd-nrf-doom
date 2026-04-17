# nRF-DOOM Build Guide

## Dependencies

### Ubuntu

These directions were written by Seth, and work for a modern Ubuntu distro (and possibly WSL too?)

1. Install common dev tools
    ```bash
    sudo apt install make get wget
    ```
1. Install the ARM Embedded Toolchain
    ```bash
    sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
    ```
1. Install JLink (https://www.segger.com/downloads/jlink)
    ```bash
    cp /mnt/c/Users/<YourWindowsUsername>/Downloads/JLink_Linux_x86_64.tar.gz .

    tar -xzf JLink_Linux_V*.tgz
    sudo mv JLink_Linux_V* /opt/jlink
    sudo ln -s /opt/jlink/JLinkExe /usr/local/bin/jlink
    ```
1. Install the `nrfutil` commandline tool
    ```bash
    wget https://developer.nordicsemi.com/.pc-tools/nrfutil/x64-linux/nrfutil
    chmod +x nrfutil
    sudo mv nrfutil /usr/local/bin/
    nrfutil install nrf5sdk-tools
    nrfutil install device
    ```
1. Fix permissions to access serial ports
    ```bash
    sudo usermod -a -G dialout $USER
    newgrp dialout
    ```

### MacOS

These directions were written by Pierson, and work for a modern MacOS version with Homebrew installed.

1. Install common dev tools
    ```bash
    brew install minicom
    ```
1. Install the ARM Embedded Toolchain
    ```bash
    brew install --cask gcc-arm-embedded
    ```
1. Install JLink, from https://www.segger.com/downloads/jlink
1. Install the nRF Command Line Tools from https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download
1. Install the `nrfutil` commandline tool
    ```bash
    curl https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/executables/universal-apple-darwin/nrfutil \
        -o /usr/local/bin/nrfutil
    chmod +x /usr/local/bin/nrfutil
    
    nrfutil install nrf5sdk-tools
    nrfutil install device
    ```

## Set up Git repository

```bash
# download repo somewhere you keep your projects
git clone https://github.com/UVicSatelliteDesign/uvsd-nrf-doom.git
cd ~/projects/uvsd-nrf-doom # or wherever your repo is
```

1. Download the nRF SDK from https://www.nordicsemi.com/Products/Development-software/nRF5-SDK/Download
    None of the SoftDevices shown are necessary.
1. Unzip the SDK into the gitignored folder `nRF5_SDK`
    ```bash
    unzip ~/Downloads/nRF5_SDK_17.0.2_d674dde.zip -d nRF5_SDK/
    ```
1. Install the `nrfx` drivers
    ```bash
    cd nRF5_SDK/modules/
    rm -rf nrfx
    git clone https://github.com/NordicSemiconductor/nrfx.git
    cd nrfx
    git checkout v2.4.0
    ```

## Required Code Changes

three fixes needed for compatibility with nrfx 2.4.0 and modern gcc:

**1. fix include paths**
```bash
cd ../../..  # Change to your repo root
sed -i 's|cmsis/Include|cmsis/include|g' nrfdoom/nrf5340dk/armgcc/Makefile
sed -i 's|cmsis/Include|cmsis/include|g' nrfdoom_net/Makefile
```

**2. fix SPI driver**

edit `nRF5_SDK/integration/nrfx/legacy/nrf_drv_spi.c` line 111

change:
```c
nrfx_spim_config_t config_spim = NRFX_SPIM_DEFAULT_CONFIG;
```

to:
```c
nrfx_spim_config_t config_spim = NRFX_SPIM_DEFAULT_CONFIG(
    NRF_DRV_SPI_PIN_NOT_USED,
    NRF_DRV_SPI_PIN_NOT_USED,
    NRF_DRV_SPI_PIN_NOT_USED,
    NRF_DRV_SPI_PIN_NOT_USED);
```

**3. fix SD card driver**
```bash
sed -i 's/nrf_spi_frequency_t/nrf_spim_frequency_t/g' nRF5_SDK/components/libraries/sdcard/app_sdcard.c
```

**4. fix Makefile**
edit `nRF5_SDK/components/toolchain/gcc/Makefile.posix`

change:
```c
GNU_INSTALL_ROOT ?= /usr/local/gcc-arm-none-eabi-7-2018-q2-update/bin/
```

to:
```c
GNU_INSTALL_ROOT ?= /usr/bin/
```

## Configure `minicom`

1. Run `minicom` with the `--setup` flag
    ```bash
    minicom --setup
    ```
1. Use arrow keys to navigate to **Screen**, then press Enter
1. Press the `R` and `T` keys to toggle **Line Wrap** and **Add Carriage Return** on
1. Press `Esc` twice to exit the setup menu

## Build

```bash
# network core first
cd nrfdoom_net
make

# then app core
cd ../nrfdoom/nrf5340dk/armgcc
make
```

warnings about _close, _fstat etc are normal

## Flash
The nRF5340 DK has two micro USB ports. Both must be connected for programming/flashing.

The debug port is placed midway along the short edge of the board. It should be connected to your PC for programming.

The nRF USB port is placed offset along the long edge of the board. It should be connected to a power source (which may or may not be your PC) to supply power to the board.

```bash
# check board detected
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
