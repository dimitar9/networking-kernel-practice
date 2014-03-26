KDIR := /lib/modules/$(shell uname -r)/build
LIB_PATH = /usr/lib
obj-m += nlexample.o
obj-m += nlexample2.o
obj-m += genlexample.o

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

userspace:
	gcc change.c /usr/lib/x86_64-linux-gnu/libmnl.so -o change

	gcc event.c /usr/lib/x86_64-linux-gnu/libmnl.so -o event
clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp* change event
