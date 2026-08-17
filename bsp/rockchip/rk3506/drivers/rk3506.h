#ifndef __RK3506_H__
#define __RK3506_H__

#include <rtdef.h>

#define RK3506_DRAM_BASE       BSP_MEMORY_BASE
#define RK3506_DRAM_SIZE       BSP_MEMORY_SIZE

#define RK3506_GIC_DIST_BASE   0xff581000U
#define RK3506_GIC_CPU_BASE    0xff582000U

#define RK3506_UART0_BASE      0xff0a0000U
#define RK3506_UART1_BASE      0xff0b0000U
#define RK3506_UART2_BASE      0xff0c0000U
#define RK3506_UART3_BASE      0xff0d0000U
#define RK3506_UART4_BASE      0xff0e0000U
#define RK3506_UART5_BASE      0xff4e0000U

#define RK3506_GIC_SPI(n)      ((n) + 32U)
#define RK3506_UART1_IRQ       RK3506_GIC_SPI(35)

#define RK3506_GENERIC_TIMER_HZ 24000000U

#ifndef __REG32
#define __REG32(x)             (*((volatile unsigned int *)(x)))
#endif

#define ARM_GIC_NR_IRQS        256
#define ARM_GIC_MAX_NR         1

#define MAX_HANDLERS           256
#define GIC_IRQ_START          0

rt_inline rt_uint32_t platform_get_gic_dist_base(void)
{
    return RK3506_GIC_DIST_BASE;
}

rt_inline rt_uint32_t platform_get_gic_cpu_base(void)
{
    return RK3506_GIC_CPU_BASE;
}

#endif
