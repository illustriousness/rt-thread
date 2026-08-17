ROOT := $(abspath ../../../..)
MKIMAGE ?= $(ROOT)/uboot/tools/mkimage

.PHONY: all image smp-image amp-f01-image clean

all: image

image: smp-image

smp-image:
	scons --useconfig=configs/smp_defconfig
	RK3506_FIRMWARE_BASE=0x04000000 scons -j$$(nproc)
	cp rtthread.bin rtthread-smp.bin
	$(MKIMAGE) -f smp.its -E -p 0xe00 rtthread-smp.img

amp-f01-image:
	scons --useconfig=configs/amp_f01_defconfig
	RK3506_FIRMWARE_BASE=0x05000000 scons -j$$(nproc)
	cp rtthread.bin rtthread-amp-f01.bin
	$(MKIMAGE) -f amp-f01.its -E -p 0xe00 rtthread-amp-f01.img

clean:
	scons -c
	rm -f rtthread-smp.img rtthread-amp-f01.img rtthread-*.bin
