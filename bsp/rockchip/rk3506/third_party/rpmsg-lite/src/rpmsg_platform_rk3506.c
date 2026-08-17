#include <rthw.h>
#include <rtthread.h>

#include "rpmsg_env.h"
#include "rpmsg_platform.h"

#define CRU_BASE                    0xff9a0000U
#define CRU_GATE_CON6               (CRU_BASE + 0x818U)
#define PCLK_MAILBOX_GATE_BIT       13U

#define MAILBOX0_BASE               0xff290000U
#define MAILBOX1_BASE               0xff291000U
#define MAILBOX1_REMOTE_IRQ         (143U + 32U)

#define MBOX_A2B_INTEN              0x00U
#define MBOX_A2B_STATUS             0x04U
#define MBOX_A2B_CMD                0x08U
#define MBOX_A2B_DATA               0x0cU
#define MBOX_B2A_INTEN              0x10U
#define MBOX_B2A_STATUS             0x14U
#define MBOX_B2A_CMD                0x18U
#define MBOX_B2A_DATA               0x1cU

#define MBOX_TX_DONE                (1U << 0)
#define MBOX_RX_DONE                (1U << 1)
#define MBOX_TRIGGER_DATA           (1U << 8)
#define MBOX_WRITE_MASK(bit)        (1U << ((bit) + 16U))
#define RPMSG_MBOX_MAGIC            0x524d5347U

#define REG32(addr)                 (*(volatile uint32_t *)(addr))

static int interrupt_disable_depth;

static void barrier(void)
{
    __asm__ volatile("dsb sy" ::: "memory");
}

static void mailbox_clock_enable(void)
{
    REG32(CRU_GATE_CON6) = MBOX_WRITE_MASK(PCLK_MAILBOX_GATE_BIT);
    barrier();
}

static void mailbox_configure(void)
{
    mailbox_clock_enable();

    REG32(MAILBOX0_BASE + MBOX_B2A_INTEN) =
        MBOX_WRITE_MASK(8U) | MBOX_TRIGGER_DATA;
    REG32(MAILBOX1_BASE + MBOX_B2A_INTEN) =
        MBOX_WRITE_MASK(8U) | MBOX_TRIGGER_DATA;

    REG32(MAILBOX1_BASE + MBOX_A2B_STATUS) =
        MBOX_TX_DONE | MBOX_RX_DONE;
    REG32(MAILBOX1_BASE + MBOX_A2B_INTEN) =
        MBOX_WRITE_MASK(0U) | MBOX_TX_DONE;
    barrier();
}

static void mailbox_isr(int vector, void *parameter)
{
    uint32_t status;

    RT_UNUSED(vector);
    RT_UNUSED(parameter);

    status = REG32(MAILBOX1_BASE + MBOX_A2B_STATUS);
    if (status & MBOX_TX_DONE)
    {
        volatile uint32_t cmd = REG32(MAILBOX1_BASE + MBOX_A2B_CMD);
        volatile uint32_t data = REG32(MAILBOX1_BASE + MBOX_A2B_DATA);

        RT_UNUSED(cmd);
        RT_UNUSED(data);
        REG32(MAILBOX1_BASE + MBOX_A2B_STATUS) = MBOX_TX_DONE;
        barrier();

        env_isr(RL_GET_VQ_ID(1U, 0U));
        env_isr(RL_GET_VQ_ID(1U, 1U));
    }

    if (status & MBOX_RX_DONE)
    {
        REG32(MAILBOX1_BASE + MBOX_A2B_STATUS) = MBOX_RX_DONE;
    }
}

int32_t platform_init_interrupt(uint32_t vector_id, void *isr_data)
{
    env_register_isr(vector_id, isr_data);
    return 0;
}

int32_t platform_deinit_interrupt(uint32_t vector_id)
{
    env_unregister_isr(vector_id);
    return 0;
}

int32_t platform_interrupt_enable(uint32_t vector_id)
{
    RT_UNUSED(vector_id);

    if (interrupt_disable_depth > 0 && --interrupt_disable_depth == 0)
    {
        mailbox_configure();
        rt_hw_interrupt_umask(MAILBOX1_REMOTE_IRQ);
    }
    return 0;
}

int32_t platform_interrupt_disable(uint32_t vector_id)
{
    RT_UNUSED(vector_id);

    if (interrupt_disable_depth++ == 0)
    {
        rt_hw_interrupt_mask(MAILBOX1_REMOTE_IRQ);
    }
    return 0;
}

void platform_notify(uint32_t vector_id)
{
    uint32_t base = RL_GET_Q_ID(vector_id) ? MAILBOX1_BASE : MAILBOX0_BASE;

    barrier();
    if (REG32(base + MBOX_B2A_STATUS) & MBOX_TX_DONE)
    {
        return;
    }

    REG32(base + MBOX_B2A_CMD) = RL_GET_LINK_ID(vector_id);
    REG32(base + MBOX_B2A_DATA) = RPMSG_MBOX_MAGIC;
}

void platform_time_delay(uint32_t num_msec)
{
    rt_thread_mdelay(num_msec);
}

void platform_poll(void)
{
    /* Linux initializes the GIC after the remote starts and masks all SPIs. */
    rt_hw_interrupt_umask(MAILBOX1_REMOTE_IRQ);
    if (REG32(MAILBOX1_BASE + MBOX_A2B_STATUS) &
        (MBOX_TX_DONE | MBOX_RX_DONE))
    {
        mailbox_isr(MAILBOX1_REMOTE_IRQ, RT_NULL);
    }
}

void platform_map_mem_region(uint32_t vrt_addr, uint32_t phy_addr,
                             uint32_t size, uint32_t flags)
{
    RT_UNUSED(vrt_addr);
    RT_UNUSED(phy_addr);
    RT_UNUSED(size);
    RT_UNUSED(flags);
}

void platform_cache_all_flush_invalidate(void)
{
}

void platform_cache_disable(void)
{
}

uintptr_t platform_vatopa(void *addr)
{
    return (uintptr_t)addr;
}

void *platform_patova(uintptr_t addr)
{
    return (void *)addr;
}

int32_t platform_init(void)
{
    interrupt_disable_depth = 0;
    mailbox_configure();
    rt_hw_interrupt_install(MAILBOX1_REMOTE_IRQ, mailbox_isr, RT_NULL,
                            "rpmsg-mbox");
    rt_hw_interrupt_mask(MAILBOX1_REMOTE_IRQ);
    return 0;
}

int32_t platform_deinit(void)
{
    rt_hw_interrupt_mask(MAILBOX1_REMOTE_IRQ);
    return 0;
}
