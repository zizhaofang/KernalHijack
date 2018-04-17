ifeq ($(KERNELRELEASE),)  

KERNELDIR ?= /lib/modules/$(shell uname -r)/build 
PWD := $(shell pwd)  

.PHONY: build clean  

build:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules  
sneaky_process:
	gcc sneaky_process.c -o sneaky_process
clean:
	rm -rf *.o *~ core .depend .*.cmd *.order *.symvers *.ko *.mod.c sneaky_process
else  

$(info Building with KERNELRELEASE = ${KERNELRELEASE}) 
obj-m :=    sneaky_mod.o  

endif
