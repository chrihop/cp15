# -*-Makefile-*-

#
# Top-level Makefile for cp15 (an ARM cp15 probe)
#

#
# build the driver runs in PL1
#

ifneq (${KERNELRELEASE},)
	obj-m := cp15.o
else
	KERNEL_SOURCE := /usr/src/linux-headers-$(shell uname -r)
	PWD := $(shell pwd)
driver:
	${MAKE} -I. -C ${KERNEL_SOURCE} SUBDIRS=${PWD} modules
clean:
	${MAKE} -I. -C ${KERNEL_SOURCE} SUBDIRS=${PWD} clean
endif

client:
	gcc -I. cp15_client.c -o cp15_client

all:
	driver client
