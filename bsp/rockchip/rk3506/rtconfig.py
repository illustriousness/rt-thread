import os

ARCH = 'arm'
CPU = 'cortex-a'
CROSS_TOOL = 'gcc'
PLATFORM = 'gcc'
EXEC_PATH = os.getenv('RTT_EXEC_PATH') or '/usr/bin'
BUILD = 'debug'

LINK_SCRIPT = 'link.lds'
FIRMWARE_BASE = os.getenv('RK3506_FIRMWARE_BASE') or '0x04000000'

if PLATFORM == 'gcc':
    PREFIX = os.getenv('RTT_CC_PREFIX') or 'arm-none-eabi-'
    CC = PREFIX + 'gcc'
    CXX = PREFIX + 'g++'
    AS = PREFIX + 'gcc'
    AR = PREFIX + 'ar'
    LINK = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY = PREFIX + 'objcopy'
    STRIP = PREFIX + 'strip'

    DEVICE = ' -march=armv7-a -mtune=cortex-a7 -msoft-float'
    DEVICE += ' -ffunction-sections -fdata-sections -funwind-tables'
    DEVICE += ' -fno-strict-aliasing -fdiagnostics-color=always'

    CFLAGS = DEVICE + ' -Wall -Wno-cpp -std=gnu99 -D_POSIX_SOURCE'
    CXXFLAGS = DEVICE + ' -Wall -Woverloaded-virtual -fno-rtti'
    AFLAGS = ' -march=armv7-a -mtune=cortex-a7 -c -x assembler-with-cpp -D__ASSEMBLY__'
    LFLAGS = DEVICE + ' -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,system_vectors'
    LFLAGS += ' -T ' + LINK_SCRIPT + ' -lgcc -static'
    LFLAGS += ' -Wl,--defsym=RTT_FIRMWARE_BASE=' + FIRMWARE_BASE
    CPATH = ''
    LPATH = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -gdwarf-2'
        CXXFLAGS += ' -O0 -gdwarf-2'
        AFLAGS += ' -gdwarf-2'
    else:
        CFLAGS += ' -Os'
        CXXFLAGS += ' -Os'

    DUMP_ACTION = OBJDUMP + ' -D -S $TARGET > rtthread.asm\n'
    POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET\n'
