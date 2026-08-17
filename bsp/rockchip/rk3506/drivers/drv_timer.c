#include <rthw.h>
#include <rtthread.h>

#include <board.h>
#include <gtimer.h>

#ifndef BSP_GENERIC_TIMER_IRQ
#define BSP_GENERIC_TIMER_IRQ 30
#endif

static rt_uint32_t timer_reload;
static volatile rt_uint32_t timer_irq_count[RT_CPUS_NR];

static void rk3506_timer_isr(int vector, void *param)
{
    RT_UNUSED(vector);
    RT_UNUSED(param);

    timer_irq_count[rt_hw_cpu_id()]++;
    rt_tick_increase();
    gtimer_set_load_value(timer_reload);
    gtimer_set_control(1);
}

void rk3506_timer_cpu_init(void)
{
    rt_uint32_t freq = gtimer_get_counter_frequency();

    if (!freq)
    {
        freq = RK3506_GENERIC_TIMER_HZ;
        gtimer_set_counter_frequency(freq);
    }

    timer_reload = freq / RT_TICK_PER_SECOND;

    gtimer_set_control(0);
    gtimer_set_load_value(timer_reload);

    rt_hw_interrupt_install(BSP_GENERIC_TIMER_IRQ, rk3506_timer_isr, RT_NULL, "tick");
    rt_hw_interrupt_umask(BSP_GENERIC_TIMER_IRQ);

    gtimer_set_control(1);
}

rt_uint32_t rk3506_timer_irq_count(int cpu)
{
    if (cpu < 0 || cpu >= RT_CPUS_NR)
    {
        return 0;
    }

    return timer_irq_count[cpu];
}

int rt_hw_timer_init(void)
{
    rk3506_timer_cpu_init();

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_timer_init);
