# HIH8121
HIH8121 Temperature &amp; Humidity sensor kernel module

Built on Ubuntu 18.04.3 for Raspberry Pi Raspbian kernel version 4.19.69-v7+  

To dynamically load the module:  
sudo insmod hih8121.ko  
echo "hih8121_i2c 0x27" | sudo tee /sys/bus/i2c/devices/i2c-1/new_device
