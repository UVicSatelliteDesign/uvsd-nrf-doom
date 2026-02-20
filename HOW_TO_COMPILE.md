# nRF-Doom Build Guide

quick guide to get nRF-Doom compiling on Ubuntu with modern toolchains

## install prereqs

```bash
# arm compiler
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi

# other tools
sudo apt install make git wget

# jlink
wget https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb
sudo dpkg -i JLink_Linux_x86_64.deb

# nrfutil
wget https://developer.nordicsemi.com/.pc-tools/nrfutil/x64-linux/nrfutil
chmod +x nrfutil
sudo mv nrfutil /usr/local/bin/
nrfutil install nrf5sdk-tools
nrfutil install device

# permissions
sudo usermod -a -G dialout $USER
newgrp dialout
```

## setup

```bash
# download repo somewhere you keep your projects
git clone https://github.com/UVicSatelliteDesign/uvsd-nrf-doom.git

```

```bash
# get SDK
cd ~/Downloads
wget https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v17.x.x/nRF5_SDK_17.0.2_d674dde.zip
cd ~/projects/uvsd-nrf-doom  # or wherever your repo is
unzip ~/Downloads/nRF5_SDK_17.0.2_d674dde.zip -d nRF5_SDK/

# get nrfx 2.4.0
cd nRF5_SDK/modules/
rm -rf nrfx
git clone https://github.com/NordicSemiconductor/nrfx.git
cd nrfx
git checkout v2.4.0
```

## required code changes

three fixes needed for compatibility with nrfx 2.4.0 and modern gcc:

**1. fix include paths**
```bash
cd ~/projects/uvsd-nrf-doom  # your repo root
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

## build

```bash
# network core first
cd nrfdoom_net
make

# then app core
cd ../nrfdoom/nrf5340dk/armgcc
make
```

warnings about _close, _fstat etc are normal

## flash

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

## test

```bash
# check UART
minicom -D /dev/ttyACM1 -b 115200

# or just cat it
cat /dev/ttyACM1

# reset board
nrfutil device reset --reset-kind RESET_SYSTEM
```

LED 3 should be on when running

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
