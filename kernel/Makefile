obj-m += hello_world.o
obj-m += datenlord2.o
obj-m += datenlord3.o
obj-m += datenlord4.o
obj-m += datenlord5.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
