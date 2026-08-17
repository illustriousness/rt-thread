#include <rtthread.h>

#include <rk3506_amp_protocol.h>

#include "amp_shm_service.h"

static volatile struct amp_shm *const amp_shm =
    (volatile struct amp_shm *)AMP_SHM_BASE;

static void amp_memory_barrier(void)
{
    __asm__ volatile ("dmb sy" ::: "memory");
}

static void amp_shm_worker(void *parameter)
{
    rt_uint32_t last_request = 0;

    RT_UNUSED(parameter);

    while (1)
    {
        rt_uint32_t request = amp_shm->request_seq;

        amp_shm->heartbeat++;
        if (request != last_request)
        {
            amp_memory_barrier();
            switch (amp_shm->opcode)
            {
            case AMP_OP_PING:
                amp_shm->result = 0x504f4e47U;
                amp_shm->error = 0;
                break;
            case AMP_OP_ADD:
                amp_shm->result = amp_shm->arg0 + amp_shm->arg1;
                amp_shm->error = 0;
                break;
            case AMP_OP_XOR:
                amp_shm->result = amp_shm->arg0 ^ amp_shm->arg1;
                amp_shm->error = 0;
                break;
            default:
                amp_shm->result = 0;
                amp_shm->error = 1;
                break;
            }

            amp_memory_barrier();
            last_request = request;
            amp_shm->response_seq = request;
        }

        rt_thread_mdelay(1);
    }
}

rt_err_t amp_shm_service_start(void)
{
    volatile rt_uint32_t *word = (volatile rt_uint32_t *)amp_shm;
    rt_thread_t thread;
    rt_size_t i;

    for (i = 0; i < sizeof(*amp_shm) / sizeof(*word); i++)
    {
        word[i] = 0;
    }

    amp_shm->magic = AMP_PROTOCOL_MAGIC;
    amp_shm->version = AMP_PROTOCOL_VERSION;
    amp_memory_barrier();
    amp_shm->state = AMP_STATE_READY;

    thread = rt_thread_create("amp-shm", amp_shm_worker, RT_NULL,
                              1024, 11, 10);
    if (thread == RT_NULL)
    {
        amp_shm->state = AMP_STATE_FAULT;
        return -RT_ENOMEM;
    }

    return rt_thread_startup(thread);
}
