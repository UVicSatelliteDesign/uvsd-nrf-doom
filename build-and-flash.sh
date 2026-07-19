#!/usr/bin/env bash
set -e

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"

# Build network core
cd "$REPO_ROOT/nrfdoom_net"
make

# Build app core
cd "$REPO_ROOT/nrfdoom/nrf5340dk/armgcc"
make

# Check board is detected
nrfutil device list

# Flash network core first
cd "$REPO_ROOT/nrfdoom_net"
make flash

# Flash app core
cd "$REPO_ROOT/nrfdoom/nrf5340dk/armgcc"
make flash
