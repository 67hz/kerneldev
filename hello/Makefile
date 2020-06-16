obj-m += hello-1.o
obj-m += startstop.o # create a name for multi-file module
startstop-objs := start.o stop.o	# which object files are part of module

all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
