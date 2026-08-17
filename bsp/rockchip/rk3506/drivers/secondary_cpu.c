#include <rthw.h>
#include <rtthread.h>

#include <board.h>
#include <drv_timer.h>
#include <gic.h>
#include <interrupt.h>

#ifdef RT_USING_SMP

#define PSCI_0_2_FN_CPU_ON  0x84000003U
#define RK3506_MPIDR_BASE   0x00000f00U

extern size_t MMUTable[];

static rt_int32_t psci_cpu_on(rt_uint32_t mpidr, rt_uint32_t entry)
{
    register rt_uint32_t r0 __asm__("r0") = PSCI_0_2_FN_CPU_ON;
    register rt_uint32_t r1 __asm__("r1") = mpidr;
    register rt_uint32_t r2 __asm__("r2") = entry;
    register rt_uint32_t r3 __asm__("r3") = 0;

    /* SMC #0 encoded explicitly because armv7-a does not imply TrustZone. */
    __asm__ volatile (".inst 0xe1600070"
                      : "+r" (r0)
                      : "r" (r1), "r" (r2), "r" (r3)
                      : "memory", "cc");

    return (rt_int32_t)r0;
}

void rt_hw_secondary_cpu_up(void)
{
    rt_uint32_t cpu;

    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)rt_secondary_cpu_entry, 64);
    rt_hw_dsb();

    for (cpu = 1; cpu < RT_CPUS_NR; cpu++)
    {
        rt_int32_t result = psci_cpu_on(RK3506_MPIDR_BASE + cpu,
                                        (rt_uint32_t)rt_secondary_cpu_entry);

        rt_kprintf("PSCI CPU_ON f0%u entry=%p result=%d\n",
                   cpu, rt_secondary_cpu_entry, result);
    }
}

void rt_hw_secondary_cpu_bsp_start(void)
{
    rt_hw_vector_init();
    rt_hw_mmu_switch((void *)MMUTable);
    arm_gic_cpu_init(0, RK3506_GIC_CPU_BASE);
    rk3506_timer_cpu_init();

    rt_hw_spin_lock(&_cpus_lock);
    rt_system_scheduler_start();
}

void rt_hw_secondary_cpu_idle_exec(void)
{
    __asm__ volatile ("wfe" ::: "memory", "cc");
}

#endif
