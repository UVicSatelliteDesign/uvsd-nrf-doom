#!/usr/bin/env bash
set -euo pipefail

Color_Off=''
Red=''
Green=''
Yellow=''
Dim='' # White
Bold_White=''
Bold_Green=''

if [[ -t 1 ]]; then
    Color_Off='\033[0m'     # Text Reset
    Red='\033[0;31m'        # Red
    Green='\033[0;32m'      # Green
    Yellow='\033[0;33m'     # Yellow
    Dim='\033[0;2m'         # White
    Bold_Green='\033[1;32m' # Bold Green
    Bold_White='\033[1m'    # Bold White
fi

verbose=""
skip_sdk=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        -v)
            verbose="true"
            shift
            ;;
        -s|--skip-sdk)
            skip_sdk="true"
            shift
            ;;
        *)
            error "Unknown argument: $1"
            ;;
    esac
done

error() {
    echo -e "${Red}error${Color_Off}:" "$@" >&2
    exit 1
}

warn() {
    echo -e "${Yellow}warn ${Color_Off}:" "$@" >&2
}

warn_check() {
    echo -e "${Yellow}warn ${Color_Off}:" "$@" >&2
    echo -ne "       ${Dim}Press ${Color_Off}Enter${Dim} to continue anyway...${Color_Off}"
    read
}

info() {
    echo -e "${Dim}info ${Color_Off}:" "$@"
}

debug() {
    if [[ -n "$verbose" ]]; then
        echo -e "${Dim}debug${Color_Off}:" "$@"
    fi
}

info_bold() {
    echo -e "${Dim}info ${Color_Off}:" "${Bold_White}$*${Color_Off}"
}

success() {
    echo -e "${Green}$*${Color_Off}"
}

if command -v usermod >/dev/null 2>&1 || [[ ! -f /usr/local/etc/minirc.dfl ]] || ! command -v nrfutil >/dev/null 2>&1; then
    info "This script requires sudo privileges for system configurations. Checking access..."
    sudo -v || error "Sudo authentication failed."
fi

temp_space=$(mktemp -d)
cleanup() {
    rm -rf "$temp_space"
}
trap cleanup EXIT

required_cmds=("git" "unzip" "sed")
recommended_cmds=("minicom" "jlink" "gcc" "make")

for item in "${required_cmds[@]}"; do
    command -v "$item" >/dev/null 2>&1 || error "$item is required but was not found."
done

if ! command -v nrfjprog >/dev/null 2>&1; then
    warn "${Bold_White}nrfjprog${Color_Off} is missing. Remember to install the nRF Command Line Tools and their bundled version of SEGGER J-Link."
fi

for item in "${recommended_cmds[@]}"; do
    command -v "$item" >/dev/null 2>&1 || warn "Remember to install ${Bold_White}${item}${Color_Off}."
done

safely_patch_file() {
    # the sed -i flag is not used because of platform inconsistencies between GNU and BSD sed

    local pattern="$1"
    local file="$2"
    sed "$pattern" "$file" > "$temp_space/patch_tmp"
    mv "$temp_space/patch_tmp" "$file"
}

download() {
    local url="$1"
    local out="$2"

    if command -v curl >/dev/null 2>&1; then
        curl --fail --location --progress-bar --output "$out" "$url"
    elif command -v wget >/dev/null 2>&1; then
        wget -qO "$out" "$url"
    else
        error "Cannot download files: neither curl nor wget is installed."
    fi
}

if command -v usermod >/dev/null 2>&1; then
    # mac doesn't have the usermod command, but also doesn't need this group

    info "Updating your group to provide access to virtual ports."
    info "Provide your password if asked."

    sudo usermod -a -G dialout "$USER"
    warn "Added to group 'dialout'. You may need to log out and back in for group changes to take effect."
fi

if ! command -v nrfutil >/dev/null 2>&1; then
    info "Installing ${Dim}nrfutil${Color_Off}"

    nrfutil_url="https://developer.nordicsemi.com/.pc-tools/nrfutil/x64-linux/nrfutil"
    download "$nrfutil_url" "$temp_space/nrfutil" || \
        error "Failed to download ${Dim}nrfutil${Color_Off} from \"$nrfutil_url\""

    chmod +x "$temp_space/nrfutil"
    sudo mv "$temp_space/nrfutil" /usr/local/bin/nrfutil
    nrfutil install nrf5sdk-tools
    nrfutil install device
fi

if [[ ! -f /usr/local/etc/minirc.dfl ]]; then
    info "Updating ${Dim}minicom${Color_Off} settings"

    sudo mkdir -p /usr/local/etc
    cat <<'EOF' | sudo tee /usr/local/etc/minirc.dfl >/dev/null
pu linewrap         Yes
pu addcarreturn     Yes
EOF
fi

git_root=$(git config --get remote.origin.url 2>/dev/null) || git_root=""

if [[ -z "$git_root" ]]; then
    warn_check "Not currently in a git directory"
elif [[ ! -d .git ]]; then
    warn_check "Not currently at the root of a git directory"
elif [[ "$git_root" != "https://github.com/UVicSatelliteDesign/uvsd-nrf-doom.git" ]]; then
    warn_check "Git origin is not the expected URL (expected ${Dim}https://github.com/UVicSatelliteDesign/uvsd-nrf-doom.git${Color_Off})"
fi

nrf_sdk_url="https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v17.x.x/nRF5_SDK_17.1.0_ddde560.zip"

if [[ -z "$skip_sdk" ]]; then
    info "Downloading nRF SDK v17.1.0"
    download "$nrf_sdk_url" "$temp_space/nrf_sdk.zip" || \
        error "Failed to download nRF SDK v17.1.0 from \"$nrf_sdk_url\""

    debug "SDK downloaded to $temp_space/nrf_sdk.zip"

    if [[ -d "nRF5_SDK" ]]; then
        warn_check "The ${Dim}./nRF5_SDK${Color_Off} directory already exists. Do you want to overwrite it?"
    fi

    rm -rf nRF5_SDK
    info "Unzipping nRF SDK v17.1.0"
    unzip -q "$temp_space/nrf_sdk.zip" -d "$temp_space/nrf_sdk_unzip"
    mv "$temp_space/nrf_sdk_unzip/nRF5_SDK_17.1.0_ddde560" "nRF5_SDK"
fi

info "Updating nRF SDK issues"


spi_file="nRF5_SDK/integration/nrfx/legacy/nrf_drv_spi.c"
spi_replacement="NRFX_SPIM_DEFAULT_CONFIG(NRF_DRV_SPI_PIN_NOT_USED, NRF_DRV_SPI_PIN_NOT_USED, NRF_DRV_SPI_PIN_NOT_USED, NRF_DRV_SPI_PIN_NOT_USED);"
debug "Updating file $spi_file"
safely_patch_file "s/NRFX_SPIM_DEFAULT_CONFIG;/${spi_replacement}/" "$spi_file"

sdcard_file="nRF5_SDK/components/libraries/sdcard/app_sdcard.c"
debug "Updating file $sdcard_file"
safely_patch_file "s/nrf_spi_frequency_t/nrf_spim_frequency_t/g" "$sdcard_file"

if ! gcc_path=$(command -v arm-none-eabi-gcc 2>/dev/null); then
    warn "Failed to find an installation of ${Dim}arm-none-eabi-gcc${Color_Off} (the ARM Embedded Toolchain)."
    warn "You will need to update ${Dim}nRF5_SDK/components/toolchain/gcc/Makefile.posix:1${Color_Off} yourself."
else
    makefile_posix="nRF5_SDK/components/toolchain/gcc/Makefile.posix"
    gcc_dirname=$(dirname "$gcc_path")
    debug "Updating file $makefile_posix to match gcc-arm-none-eabi install location $gcc_dirname"
    safely_patch_file "s|GNU_INSTALL_ROOT \?= /usr/local/gcc-arm-none-eabi-.*/bin/|GNU_INSTALL_ROOT \?= ${gcc_dirname}/|" "$makefile_posix"
fi

info "Updating ${Dim}nrfx${Color_Off} drivers"
(
    cd "nRF5_SDK/modules"
    rm -rf nrfx
    git clone -c advice.detachedHead=false -b "v2.4.0" --single-branch "https://github.com/NordicSemiconductor/nrfx.git"
)

success "Initialised your nrf-doom setup"
