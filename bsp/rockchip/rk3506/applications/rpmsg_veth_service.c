#include <rtthread.h>

#include "rpmsg_lite.h"
#include "rpmsg_ns.h"
#include "rpmsg_veth_service.h"

#define RPMSG_SHMEM_BASE       ((void *)0x03a00000U)
#define RPMSG_LINK_ID          1U
#define RPMSG_VETH_EPT_ADDR    0x500U
#define RPMSG_VETH_MTU         496U

static const rt_uint8_t remote_mac[6] = { 0x02, 0x52, 0x54, 0x00, 0x01, 0x01 };
static const rt_uint8_t remote_ip[4] = { 192, 168, 100, 101 };
static struct rpmsg_lite_instance *rpmsg_instance;
static struct rpmsg_lite_endpoint *veth_endpoint;
static rt_uint8_t reply_frame[RPMSG_VETH_MTU];

static rt_uint16_t get_be16(const rt_uint8_t *data)
{
    return ((rt_uint16_t)data[0] << 8) | data[1];
}

static void put_be16(rt_uint8_t *data, rt_uint16_t value)
{
    data[0] = (rt_uint8_t)(value >> 8);
    data[1] = (rt_uint8_t)value;
}

static rt_uint16_t checksum(const rt_uint8_t *data, rt_size_t length)
{
    rt_uint32_t sum = 0;

    while (length >= 2)
    {
        sum += get_be16(data);
        data += 2;
        length -= 2;
    }
    if (length)
    {
        sum += (rt_uint16_t)data[0] << 8;
    }
    while (sum >> 16)
    {
        sum = (sum & 0xffffU) + (sum >> 16);
    }
    return (rt_uint16_t)~sum;
}

static void swap_bytes(rt_uint8_t *left, rt_uint8_t *right, rt_size_t length)
{
    rt_size_t index;

    for (index = 0; index < length; index++)
    {
        rt_uint8_t value = left[index];
        left[index] = right[index];
        right[index] = value;
    }
}

static int send_arp_reply(const rt_uint8_t *frame, rt_uint32_t length,
                          rt_uint32_t destination)
{
    rt_uint8_t *arp;

    if (length < 42U)
    {
        return RL_RELEASE;
    }

    arp = reply_frame + 14;
    if (get_be16(frame + 12) != 0x0806U ||
        get_be16(frame + 14) != 1U ||
        get_be16(frame + 16) != 0x0800U ||
        get_be16(frame + 20) != 1U ||
        rt_memcmp(frame + 38, remote_ip, sizeof(remote_ip)) != 0)
    {
        return RL_RELEASE;
    }

    rt_memcpy(reply_frame, frame, 42);
    rt_memcpy(reply_frame, frame + 6, 6);
    rt_memcpy(reply_frame + 6, remote_mac, 6);
    put_be16(arp + 6, 2U);
    rt_memcpy(arp + 18, frame + 22, 6);
    rt_memcpy(arp + 24, frame + 28, 4);
    rt_memcpy(arp + 8, remote_mac, 6);
    rt_memcpy(arp + 14, remote_ip, 4);

    (void)rpmsg_lite_send(rpmsg_instance, veth_endpoint, destination,
                          (char *)reply_frame, 42U, RL_DONT_BLOCK);
    return RL_RELEASE;
}

static int send_icmp_reply(const rt_uint8_t *frame, rt_uint32_t length,
                           rt_uint32_t destination)
{
    rt_uint8_t *ip;
    rt_uint8_t *icmp;
    rt_uint16_t ip_length;
    rt_uint32_t ip_header_length;
    rt_uint32_t icmp_length;

    if (length < 42U || get_be16(frame + 12) != 0x0800U)
    {
        return RL_RELEASE;
    }

    ip_header_length = (frame[14] & 0x0fU) * 4U;
    if (ip_header_length < 20U || 14U + ip_header_length + 8U > length ||
        frame[23] != 1U ||
        rt_memcmp(frame + 30, remote_ip, sizeof(remote_ip)) != 0)
    {
        return RL_RELEASE;
    }

    ip_length = get_be16(frame + 16);
    if (ip_length < ip_header_length + 8U || 14U + ip_length > length ||
        frame[14U + ip_header_length] != 8U)
    {
        return RL_RELEASE;
    }

    rt_memcpy(reply_frame, frame, 14U + ip_length);
    swap_bytes(reply_frame, reply_frame + 6, 6);
    ip = reply_frame + 14;
    swap_bytes(ip + 12, ip + 16, 4);
    ip[8] = 64U;
    put_be16(ip + 10, 0U);
    put_be16(ip + 10, checksum(ip, ip_header_length));

    icmp = ip + ip_header_length;
    icmp_length = ip_length - ip_header_length;
    icmp[0] = 0U;
    put_be16(icmp + 2, 0U);
    put_be16(icmp + 2, checksum(icmp, icmp_length));

    (void)rpmsg_lite_send(rpmsg_instance, veth_endpoint, destination,
                          (char *)reply_frame, 14U + ip_length,
                          RL_DONT_BLOCK);
    return RL_RELEASE;
}

static int32_t veth_rx_callback(void *payload, uint32_t payload_len,
                                uint32_t src, void *priv)
{
    const rt_uint8_t *frame = payload;

    RT_UNUSED(priv);
    if (payload_len < 14U || payload_len > RPMSG_VETH_MTU)
    {
        return RL_RELEASE;
    }

    if (get_be16(frame + 12) == 0x0806U)
    {
        return send_arp_reply(frame, payload_len, src);
    }
    return send_icmp_reply(frame, payload_len, src);
}

static void rpmsg_veth_entry(void *parameter)
{
    RT_UNUSED(parameter);

    rpmsg_instance = rpmsg_lite_remote_init(RPMSG_SHMEM_BASE,
                                             RPMSG_LINK_ID, RL_NO_FLAGS);
    if (rpmsg_instance == RL_NULL)
    {
        rt_kprintf("rpmsg: remote init failed\n");
        return;
    }

    while (!rpmsg_lite_is_link_up(rpmsg_instance))
    {
        (void)rpmsg_lite_wait_for_link_up(rpmsg_instance, 1000U);
    }

    veth_endpoint = rpmsg_lite_create_ept(rpmsg_instance,
                                           RPMSG_VETH_EPT_ADDR,
                                           veth_rx_callback, RT_NULL);
    if (veth_endpoint == RL_NULL)
    {
        rt_kprintf("rpmsg: veth endpoint creation failed\n");
        return;
    }

    if (rpmsg_ns_announce(rpmsg_instance, veth_endpoint, "rpmsg_veth",
                          RL_NS_CREATE) != RL_SUCCESS)
    {
        rt_kprintf("rpmsg: veth announcement failed\n");
        return;
    }

    rt_kprintf("rpmsg: veth ready at 192.168.100.101 endpoint 0x500\n");
    while (1)
    {
        rt_thread_mdelay(1000);
    }
}

rt_err_t rpmsg_veth_service_start(void)
{
    rt_thread_t thread = rt_thread_create("rpmsg-veth", rpmsg_veth_entry,
                                           RT_NULL, 4096, 11, 10);

    if (thread == RT_NULL)
    {
        return -RT_ENOMEM;
    }
    return rt_thread_startup(thread);
}
