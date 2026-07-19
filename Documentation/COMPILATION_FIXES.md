# nRF-Doom Compilation Fixes for Modern Toolchains

## Overview
This document describes the changes needed to compile nRF-Doom with nrfx 2.4.0 and modern ARM GCC toolchains (tested with GCC 15.2.0 on Fedora 43).

## Problem
The original nRF-Doom project was built for nRF5 SDK 17.0.2 with an older version of nrfx. When using nrfx 2.4.0 (required for nRF5340 support), several compatibility issues arise with the legacy driver layer.

## Required Changes

### 1. Fix CMSIS Include Path (Both Makefiles)

**Files to modify:**
- `nrfdoom/nrf5340dk/armgcc/Makefile`
- `nrfdoom_net/Makefile`

**Change:**
```diff
- $(SDK_ROOT)/components/toolchain/cmsis/Include \
+ $(SDK_ROOT)/components/toolchain/cmsis/include \
```

**Reason:** The SDK uses lowercase `include` directory name.

---

### 2. Patch Legacy SPI Driver for nrfx 2.4.0

**File to modify:**
`nRF5_SDK/integration/nrfx/legacy/nrf_drv_spi.c`

**Location:** Line 111 (in function `nrf_drv_spi_init`)

**Change:**
```diff
- nrfx_spim_config_t config_spim = NRFX_SPIM_DEFAULT_CONFIG;
+ nrfx_spim_config_t config_spim = NRFX_SPIM_DEFAULT_CONFIG(
+     NRF_DRV_SPI_PIN_NOT_USED,
+     NRF_DRV_SPI_PIN_NOT_USED,
+     NRF_DRV_SPI_PIN_NOT_USED,
+     NRF_DRV_SPI_PIN_NOT_USED);
```

**Reason:** In nrfx 2.4.0, `NRFX_SPIM_DEFAULT_CONFIG` changed from a simple macro to a function-like macro that requires pin parameters. The legacy driver wasn't updated to match this change. The dummy pin values are immediately overwritten by the following lines in the function.

---

### 3. Fix SD Card Driver Type Cast

**File to modify:**
`nRF5_SDK/components/libraries/sdcard/app_sdcard.c`

**Location:** Line 254 (in function `sdc_spi_hispeed`)

**Change:**
```diff
- (nrf_spi_frequency_t) APP_SDCARD_FREQ_DATA);
+ (nrf_spim_frequency_t) APP_SDCARD_FREQ_DATA);
```

**Reason:** The code uses the SPIM (SPI Master with EasyDMA) peripheral, not the basic SPI peripheral. The type name in nrfx 2.4.0 was updated to reflect this distinction.

---

## Build Instructions

### Prerequisites
- GNU Arm Embedded Toolchain (tested with 15.2.0)
- Make
- Segger J-Link Software
- nrfutil (with nrf5sdk-tools: `nrfutil install nrf5sdk-tools`)
- nRF5 SDK 17.0.2 installed in `nRF5_SDK/`
- nrfx 2.4.0 installed in `nRF5_SDK/modules/nrfx/` (replacing bundled version)

### Compile Network Processor
```bash
cd nrfdoom_net
make
```

### Compile Application Processor
```bash
cd nrfdoom/nrf5340dk/armgcc
make
```

### Flash (requires connected nRF5340 DK)
```bash
# Network processor (must be flashed first)
cd nrfdoom_net
make flash

# Application processor
cd nrfdoom/nrf5340dk/armgcc
make flash
```

**Note:** After flashing the network processor, you must always reprogram the application processor.

---

## What Each Component Does

### Network Processor (`nrfdoom_net`)
- Runs on the nRF5340's Network Core (Cortex-M33 @ 64MHz)
- Handles wireless gamepad communication via Nordic proprietary radio
- Communicates with BBC micro:bit gamepad
- Updates LED matrix on micro:bit to show Doom guy's face

### Application Processor (`nrfdoom/nrf5340dk/armgcc`)
- Runs on the nRF5340's Application Core (Cortex-M33 @ 128MHz)
- Executes all game logic, rendering, and sound processing
- Manages display output (320x200 @ 30-36fps via SPI to FT810)
- Handles I2S audio output to DAC
- Loads WAD files from SD card to external QSPI flash
- Uses 512KB RAM + 8MB external QSPI flash + 1MB internal flash

---

## Build Output
Successful compilation produces:
- `_build/doom_nrf53p3.hex` - Application firmware
- `_build/doom_nrf53p3.bin` - Binary format
- Text size: ~300KB, Data: ~7KB, BSS: ~252KB

---

## Troubleshooting

### "nrfjprog: command not found"
Install nrf5sdk-tools:
```bash
nrfutil install nrf5sdk-tools
```

### Linker warnings about unimplemented functions
Warnings about `_close`, `_fstat`, `_lseek`, `_open`, etc. are normal for bare-metal embedded systems. These POSIX file system functions aren't implemented and won't affect functionality.

### "Unknown device" errors
Make sure you're using nrfx 2.4.0 or later. Earlier versions don't support the nRF5340.

---

## Version Information
- **nRF5 SDK:** 17.0.2
- **nrfx:** 2.4.0 (November 2020)
- **Target MCU:** nRF5340 (dual-core Cortex-M33)
- **Tested ARM GCC:** 15.2.0
- **Tested OS:** Fedora 43 Linux
