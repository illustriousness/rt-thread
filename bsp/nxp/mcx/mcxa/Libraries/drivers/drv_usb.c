/*
 * Copyright (c) 2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-19     Claude       Note: USB driver is implemented in CherryUSB port layer
 *                             This file is kept for potential future BSP-specific customizations
 */

/*
 * MCXA156 USB Support Information:
 *
 * The MCXA156 USB driver is fully implemented in the CherryUSB port layer:
 * - USB hardware driver: components/drivers/usb/cherryusb/port/kinetis/usb_dc_kinetis.c
 * - MCX glue code: components/drivers/usb/cherryusb/port/kinetis/usb_glue_mcx.c
 *
 * Configuration:
 * - Enable in menuconfig: RT-Thread Components → Device Drivers → CherryUSB
 * - Select: Device → Kinetis MCX
 * - USB Base Address: 0x400A4000
 * - IRQ: USB0_IRQn (36)
 *
 * No additional BSP-level driver code is needed for basic USB functionality.
 */
