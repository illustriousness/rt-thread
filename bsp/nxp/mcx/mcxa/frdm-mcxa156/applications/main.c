/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-24     Magicoe      first version
 * 2020-01-10     Kevin/Karl   Add PS demo
 * 2020-09-21     supperthomas fix the main.c 6
 *
 */

#include <rtdevice.h>
#include "drv_pin.h"
#include "finsh.h"

#ifdef RT_USING_LWIP
#include <lwip/dns.h>
#include <netdev.h>

/*
 * USB RNDIS Network Configuration (DHCP Client Mode)
 *
 * Modified to use DHCP CLIENT mode: Board gets IP from PC
 * The board will request IP from PC's DHCP server via USB RNDIS
 *
 * Network topology:
 * Internet → PC (with ICS/NAT) → USB RNDIS → Board (DHCP Client)
 *
 * Setup on PC:
 * 1. Windows: Enable "Internet Connection Sharing" on your Wi-Fi/Ethernet
 *    - This will make PC act as DHCP server on RNDIS interface
 *    - PC will typically get IP 192.168.137.1
 *    - Board will get IP 192.168.137.x from PC
 *
 * 2. Linux: Install and configure dnsmasq as DHCP server on RNDIS interface
 *    - Or use NetworkManager with connection sharing
 */

/* Display USB RNDIS network status */
static int usb_network_status(void)
{
    struct netdev *netdev = netdev_get_by_name("u0");

    if (netdev == RT_NULL)
    {
        rt_kprintf("USB network interface 'u0' not found!\n");
        rt_kprintf("Make sure USB cable is connected and RNDIS driver is loaded on PC.\n");
        return -1;
    }

    rt_kprintf("\n========== USB RNDIS Network Status (DHCP Client Mode) ==========\n");
    rt_kprintf("Interface: u0\n");
    rt_kprintf("Link:      %s\n", netdev_is_link_up(netdev) ? "UP" : "DOWN");
    rt_kprintf("Status:    %s\n", netdev_is_up(netdev) ? "UP" : "DOWN");
    rt_kprintf("IP:        %s\n", ipaddr_ntoa(&netdev->ip_addr));
    rt_kprintf("Netmask:   %s\n", ipaddr_ntoa(&netdev->netmask));
    rt_kprintf("Gateway:   %s\n", ipaddr_ntoa(&netdev->gw));
    rt_kprintf("DNS #0:    %s\n", ipaddr_ntoa(&netdev->dns_servers[0]));
    rt_kprintf("DNS #1:    %s\n", ipaddr_ntoa(&netdev->dns_servers[1]));
    rt_kprintf("\n");
    rt_kprintf("Board is DHCP CLIENT - getting IP from PC\n");
    rt_kprintf("PC should be at: 192.168.137.1 (gateway)\n");
    rt_kprintf("\n");
    rt_kprintf("Setup steps for internet access:\n");
    rt_kprintf("  1. Connect USB cable to PC\n");
    rt_kprintf("  2. On PC, enable Internet Connection Sharing (ICS):\n");
    rt_kprintf("     - Windows: Network properties → Sharing → Allow ICS\n");
    rt_kprintf("     - Linux: Install dnsmasq or use NetworkManager sharing\n");
    rt_kprintf("  3. Board will auto-get IP via DHCP from PC\n");
    rt_kprintf("\n");
    rt_kprintf("Test commands:\n");
    rt_kprintf("  ping 192.168.137.1    # Ping PC (gateway)\n");
    rt_kprintf("  ping 8.8.8.8          # Ping Internet DNS\n");
    rt_kprintf("  ping www.baidu.com    # Test DNS resolution\n");
    rt_kprintf("=================================================================\n\n");

    return 0;
}
// MSH_CMD_EXPORT(usb_network_status, Show USB RNDIS network status);

/* Manual DNS configuration (usually not needed in DHCP client mode) */
static int set_dns_manual(void)
{
    struct netdev *netdev = netdev_get_by_name("u0");
    ip_addr_t dns_server;

    if (netdev == RT_NULL || !netdev_is_link_up(netdev))
    {
        rt_kprintf("USB network interface not ready!\n");
        return -1;
    }
    netdev_set_if("u0", "192.168.137.2", "192.168.137.1", "255.255.255.0");
    rt_kprintf("Manually configuring DNS servers...\n");
    rt_kprintf("Note: DHCP should auto-configure DNS. Only use this if DNS is not working.\n\n");

    /* Use PC's IP as DNS server (PC will forward DNS queries to internet) */
    IP4_ADDR(&dns_server, 192, 168, 137, 1);
    netdev_low_level_set_dns_server(netdev, 0, &dns_server);
    rt_kprintf("Primary DNS:   192.168.137.1 (via PC gateway)\n");

    /* Use Google DNS as backup */
    IP4_ADDR(&dns_server, 8, 8, 8, 8);
    netdev_low_level_set_dns_server(netdev, 1, &dns_server);
    rt_kprintf("Secondary DNS: 8.8.8.8 (Google DNS)\n");

    rt_kprintf("\nDNS configured! Now you can:\n");
    rt_kprintf("  ping www.baidu.com\n");
    rt_kprintf("  ping www.google.com\n");

    return 0;
}
MSH_CMD_EXPORT(set_dns_manual, Manually set DNS servers (for troubleshooting));

#endif /* RT_USING_LWIP */

/*
 * =================================================================
 * NTP and MQTT Package Usage Guide
 * =================================================================
 *
 * The project has been configured to support NTP and MQTT packages.
 * However, the package source code needs to be properly integrated.
 *
 * To完成integrate these packages:
 *
 * Method 1: Using ENV tool (Recommended for Windows)
 * ---------------------------------------------------
 * 1. Open RT-Thread Env console in BSP directory
 * 2. Run: menuconfig
 * 3. Navigate to: RT-Thread online packages → IoT
 * 4. Enable: [*] paho-mqtt and [*] netutils
 * 5. Save and exit
 * 6. Run: pkgs --update
 * 7. Run: scons --target=mdk5
 * 8. Rebuild: scons -j8
 *
 * Method 2: Manual integration (Current Status)
 * ---------------------------------------------
 * Packages have been downloaded to ./packages/ directory:
 * - packages/pahomqtt/     (MQTT client library)
 * - packages/netutils/     (NTP, iperf, tftp tools)
 * - packages/cJSON/        (JSON parser)
 *
 * After proper integration, you can use:
 *
 * NTP Commands:
 * -------------
 * msh> ntp_sync                    # Sync time from NTP server
 * msh> date                         # Show current date/time
 *
 * MQTT Example (after integration):
 * ----------------------------------
 * Connect to public MQTT broker:
 *   mqtt_connect tcp://broker.emqx.io:1883 client_id
 * Subscribe to topic:
 *   mqtt_subscribe topic_name
 * Publish message:
 *   mqtt_publish topic_name "message"
 *
 * Quick Test After Network is UP:
 * -------------------------------
 * 1. Test network: ping 8.8.8.8
 * 2. Sync time: ntp_sync
 * 3. Check time: date
 * 4. Connect MQTT broker
 * 5. Publish/Subscribe messages
 *
 * =================================================================
 */

#define LED_PIN ((3 * 32) + 12)

void func(void *parameter)
{
    rt_kprintf("build %s %s\r\n", __DATE__, __TIME__);
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);  /* Set GPIO as Output */
    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);    /* Set GPIO output 1 */
        rt_thread_mdelay(500);               /* Delay 500mS */
        rt_pin_write(LED_PIN, PIN_LOW);     /* Set GPIO output 0 */
        rt_thread_mdelay(500);               /* Delay 500mS */
        // rt_kprintf("MCXA156 HelloWorld\r\n");
    }
}

int main(void)
{
    rt_thread_t tid = rt_thread_create("main", func, 0, 512, 30, 5);
    if (tid)
    {
        rt_thread_startup(tid);
    }
    return 0;
}

// end file
