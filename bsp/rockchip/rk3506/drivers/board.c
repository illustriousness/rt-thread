#include <rthw.h>
#include <rtthread.h>

#include <board.h>

#ifdef RT_USING_SMP
extern void rt_hw_ipi_handler_install(int ipi_vector, rt_isr_handler_t handler);
#endif

extern size_t MMUTable[];

struct mem_desc platform_mem_desc[] =
{
    { 0x00000000, 0x04000000, 0x00000000, DEVICE_MEM },
    { RK3506_DRAM_BASE, RK3506_DRAM_BASE + RK3506_DRAM_SIZE, RK3506_DRAM_BASE, NORMAL_MEM },
    { 0xff000000, 0xffffffff, 0xff000000, DEVICE_MEM },
};

const rt_uint32_t platform_mem_desc_size = sizeof(platform_mem_desc) / sizeof(platform_mem_desc[0]);

static void idle_wfi(void)
{
    __asm__ volatile ("wfi");
}

int rt_hw_cpu_id_early(void)
{
    rt_uint32_t mpidr;

    __asm__ volatile ("mrc p15, 0, %0, c0, c0, 5" : "=r" (mpidr));
    return (mpidr & 0xfU) - BSP_CPU_AFF0_BASE;
}

int rt_hw_cpu_id(void)
{
    return rt_hw_cpu_id_early();
}

void rt_hw_board_init(void)
{
    rt_hw_mmu_map_init(&rt_kernel_space, (void *)0x80000000, 0x10000000, MMUTable, 0);
    rt_hw_init_mmu_table(platform_mem_desc, platform_mem_desc_size);
    rt_hw_mmu_init();
    rt_hw_mmu_ioremap_init(&rt_kernel_space, (void *)0x80000000, 0x10000000);

    rt_system_heap_init(HEAP_BEGIN, HEAP_END);

    rt_hw_interrupt_init();

#ifdef RT_USING_SMP
    rt_hw_ipi_handler_install(RT_SCHEDULE_IPI, rt_scheduler_ipi_handler);
#endif

    rt_components_board_init();

#if defined(RT_USING_CONSOLE) && defined(RT_CONSOLE_DEVICE_NAME)
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    rt_thread_idle_sethook(idle_wfi);
}
