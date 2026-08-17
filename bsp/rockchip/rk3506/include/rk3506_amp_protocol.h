#ifndef RK3506_AMP_PROTOCOL_H
#define RK3506_AMP_PROTOCOL_H

#include <stdint.h>

#define AMP_SHM_BASE          0x03900000u
#define AMP_SHM_SIZE          0x00100000u
#define AMP_PROTOCOL_MAGIC    0x414d5031u
#define AMP_PROTOCOL_VERSION  1u

enum amp_state
{
    AMP_STATE_RESET = 0,
    AMP_STATE_READY = 1,
    AMP_STATE_FAULT = 2,
};

enum amp_opcode
{
    AMP_OP_PING = 1,
    AMP_OP_ADD = 2,
    AMP_OP_XOR = 3,
};

struct amp_shm
{
    uint32_t magic;
    uint32_t version;
    uint32_t state;
    uint32_t heartbeat;
    uint32_t request_seq;
    uint32_t response_seq;
    uint32_t opcode;
    uint32_t arg0;
    uint32_t arg1;
    uint32_t result;
    uint32_t error;
    uint32_t reserved[5];
};

_Static_assert(sizeof(struct amp_shm) == 64, "AMP protocol must fit one cache line");

#endif
