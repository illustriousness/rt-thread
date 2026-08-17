#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtconfig.h>
#include <rk3506.h>
#include <mmu.h>

#if defined(__GNUC__)
extern int __bss_end;
#define HEAP_BEGIN      ((void *)&__bss_end)
#endif

#define HEAP_END        ((void *)(RK3506_DRAM_BASE + RK3506_DRAM_SIZE))

void rt_hw_board_init(void);

#endif
