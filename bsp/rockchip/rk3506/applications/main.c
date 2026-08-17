#include <rtthread.h>
#include <drv_timer.h>

#ifdef BSP_USING_AMP_SHM
#include "amp_shm_service.h"
#endif

#ifdef BSP_USING_RPMSG_VETH
#include "rpmsg_veth_service.h"
#endif

static void cpu_worker(void *parameter)
{
    int expected_cpu = (int)(rt_ubase_t)parameter;

    while (1)
    {
        int cpu = rt_hw_cpu_id();

#ifdef RT_USING_SMP
        rt_kprintf("SMP cpu=%d expected=%d tick=%u timer_irqs=%u\n",
#else
        rt_kprintf("AMP cpu=%d expected=%d tick=%u timer_irqs=%u\n",
#endif
                   cpu, expected_cpu, rt_tick_get(),
                   rk3506_timer_irq_count(cpu));
        rt_thread_mdelay(1000);
    }
}

int main(void)
{
    int cpu;

#ifdef RT_USING_SMP
    rt_kprintf("RK3506 open RT-Thread SMP bring-up: %d CPUs\n", RT_CPUS_NR);
#else
    rt_kprintf("RK3506 open RT-Thread AMP bring-up: physical f%02x as CPU0\n",
               BSP_CPU_AFF0_BASE);
#endif

#ifdef BSP_USING_AMP_SHM
    if (amp_shm_service_start() != RT_EOK)
    {
        rt_kprintf("failed to start AMP shared-memory service\n");
    }
#endif

#ifdef BSP_USING_RPMSG_VETH
    if (rpmsg_veth_service_start() != RT_EOK)
    {
        rt_kprintf("failed to start RPMsg veth service\n");
    }
#endif

    for (cpu = 0; cpu < RT_CPUS_NR; cpu++)
    {
        char name[RT_NAME_MAX];
        rt_thread_t thread;

        rt_snprintf(name, sizeof(name), "cpu%d", cpu);
        thread = rt_thread_create(name, cpu_worker, (void *)(rt_ubase_t)cpu,
                                  2048, 12, 10);
        if (thread == RT_NULL)
        {
            rt_kprintf("failed to create worker for CPU %d\n", cpu);
            continue;
        }

        rt_thread_control(thread, RT_THREAD_CTRL_BIND_CPU,
                          (void *)(rt_ubase_t)cpu);
        rt_thread_startup(thread);
    }

    return 0;
}
