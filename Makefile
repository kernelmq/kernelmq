export CCPREFIX = i686-elf-

export AR = $(CCPREFIX)ar
export AS = $(CCPREFIX)as
export CC = $(CCPREFIX)gcc

run: run-iso

all:     all-kernel   all-iso   all-libk
clean: clean-kernel clean-iso clean-libk

all-kernel: all-libk
	make all -C arch I=$(shell pwd)/include LIBK=$(shell pwd)/libk/libk.a

clean-kernel:
	make clean -C arch

run-iso: all-kernel
	make run -C iso K=$(shell pwd)/arch/kernel

all-iso: all-kernel
	make all -C iso K=$(shell pwd)/arch/kernel

clean-iso:
	make clean -C iso

all-libk:
	make all -C libk

clean-libk:
	make clean -C libk
