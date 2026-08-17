#ifndef RPMSG_PLATFORM_H_
#define RPMSG_PLATFORM_H_

#include <stdint.h>

#define VRING_ALIGN                    (0x1000U)
#define VRING_SIZE                     (0x8000UL)
#define RL_VRING_OVERHEAD              (2UL * VRING_SIZE)
#define RL_GET_VQ_ID(link_id, queue_id) \
    (((queue_id) & 1U) | (((link_id) << 1U) & 0xfffffffeU))
#define RL_GET_LINK_ID(id)             (((id) & 0xfffffffeU) >> 1U)
#define RL_GET_Q_ID(id)                ((id) & 1U)
#define RL_PLATFORM_USER_LINK_ID       (0U)
#define RL_PLATFORM_HIGHEST_LINK_ID    (15U)

int32_t platform_init_interrupt(uint32_t vector_id, void *isr_data);
int32_t platform_deinit_interrupt(uint32_t vector_id);
int32_t platform_interrupt_enable(uint32_t vector_id);
int32_t platform_interrupt_disable(uint32_t vector_id);
void platform_notify(uint32_t vector_id);
void platform_time_delay(uint32_t num_msec);
void platform_poll(void);
void platform_map_mem_region(uint32_t vrt_addr, uint32_t phy_addr,
                             uint32_t size, uint32_t flags);
void platform_cache_all_flush_invalidate(void);
void platform_cache_disable(void);
uintptr_t platform_vatopa(void *addr);
void *platform_patova(uintptr_t addr);
int32_t platform_init(void);
int32_t platform_deinit(void);

#endif
