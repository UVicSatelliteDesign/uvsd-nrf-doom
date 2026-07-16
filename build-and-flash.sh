# network core first
cd nrfdoom_net
make

# then app core
cd ../nrfdoom/nrf5340dk/armgcc
make

# Flash Board

# check board detected
nrfutil device list

# flash network first
cd nrfdoom_net
make flash

# then app
cd ../nrfdoom/nrf5340dk/armgcc
make flash
