ROOT := $(abspath ../../../..)
UBOOT_MKIMAGE := $(ROOT)/uboot/tools/mkimage
SDK_MKIMAGE := $(ROOT)/rk3506_linux_1.7.6/rkbin/tools/mkimage
MKIMAGE ?= $(if $(wildcard $(UBOOT_MKIMAGE)),$(UBOOT_MKIMAGE),$(SDK_MKIMAGE))

.PHONY: all image smp-image amp-f01-image check-mkimage clean

all: image

image: smp-image

smp-image: check-mkimage
	scons --useconfig=configs/smp_defconfig
	RK3506_FIRMWARE_BASE=0x04000000 scons -j$$(nproc)
	cp rtthread.bin rtthread-smp.bin
	$(MKIMAGE) -f smp.its -E -p 0xe00 rtthread-smp.img

amp-f01-image: check-mkimage
	scons --useconfig=configs/amp_f01_defconfig
	RK3506_FIRMWARE_BASE=0x05000000 scons -j$$(nproc)
	cp rtthread.bin rtthread-amp-f01.bin
	$(MKIMAGE) -f amp-f01.its -E -p 0xe00 rtthread-amp-f01.img

check-mkimage:
	@test -x "$(MKIMAGE)" || { \
		echo "mkimage 不存在，请运行根仓库 tools/bootstrap.sh 恢复 Linux SDK" >&2; \
		exit 1; \
	}

clean:
	scons -c
	rm -f rtthread-smp.img rtthread-amp-f01.img rtthread-*.bin
