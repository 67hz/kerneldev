# defined in /src/`uname -r`/Makefile
WARN	 := -W -Wall -Wstrict-prototypes -Wmissing-prototypes -02 -fomit-frame-pointer -std=gnu89

KERNELDIR   := /lib/modules/$(shell uname -r)/build

INCLUDE := -isystem $(KERNELDIR)/include

# defined in /src/`uname -r`/Makefile
CFLAGS := -O2 -DMODULE -D__KERNEL__ $(WARN) $(INCLUDE)

obj-m = $(MOD).o

all:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) modules


.PHONY: clean

clean:
	rm -rf $(TARGET).o


pull-dev:
	scp ubuntu@`uvt-kvm ip kerneldev`:~/sandbox/* .
