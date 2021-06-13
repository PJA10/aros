DEFAULT_HOST!=../default-host.sh
HOST?=DEFAULT_HOST
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -Xlinker -Map=aros.map -ffreestanding -Wall -Wextra -mgeneral-regs-only -fstack-protector
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

ARCHDIR=arch/$(HOSTARCH)
DRIVERDIR=driver
KERNELDIR=kernel

include $(ARCHDIR)/make.config
include $(DRIVERDIR)/make.config
include $(KERNELDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS) $(KERNEL_DRIVER_CFLAGS) $(KERNEL_KERNEL_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS) $(KERNEL_DRIVER_CPPFLAGS) $(KERNEL_KERNEL_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS) $(KERNEL_DRIVER_LDFLAGS) $(KERNEL_KERNEL_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS) $(KERNEL_DRIVER_LIBS) $(KERNEL_KERNEL_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
$(KERNEL_DRIVER_OBJS) \
$(KERNEL_KERNEL_OBJS) \

# $(info    KERNEL_OBJS is $(KERNEL_OBJS)) # print KERNEL_OBJS

OBJS=\
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o \

LINK_LIST=\
$(KERNEL_OBJS) \
$(LDFLAGS) \
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(LIBS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o \


.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: aros.kernel

aros.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) -z max-page-size=4096 $(LINK_LIST)
	grub-file --is-x86-multiboot aros.kernel

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f aros.kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d
	rm -f aros.map

install: install-headers create-offsets-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: aros.kernel
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp aros.kernel $(DESTDIR)$(BOOTDIR)


create-offsets-headers: 
	$(CC) -c kernel/TCB_offsets.c $(CFLAGS) $(CPPFLAGS) -S -o - | gawk '($$1 == "->") { print "#define " $$2 " " substr($$3,2) }' > include/kernel/TCB_offsets.h
	$(CC) -c arch/i386/tss_offsets.c $(CFLAGS) $(CPPFLAGS) -S -o - | gawk '($$1 == "->") { print "#define " $$2 " " substr($$3,2) }' > include/arch-i386/tss_offsets.h

-include $(OBJS:.o=.d)
