obj-m += hih8121.o

PWD  := $(shell pwd)
ARCH := arm
KDIR := /home/alex/devel/build_pi/kernel/servpi/servpi_kernel/build
CROSS_COMPILE :=  /home/alex/devel/build_pi/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
