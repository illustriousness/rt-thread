#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

int rt_hw_timer_init(void);
void rk3506_timer_cpu_init(void);
rt_uint32_t rk3506_timer_irq_count(int cpu);

#endif
