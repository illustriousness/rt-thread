/*
 * Copyright (c) 2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-19     Claude       first version for MCXA156 USB support
 */

#ifndef __DRV_USB_H__
#define __DRV_USB_H__

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize USB clock
 */
void usb_clock_init(void);

/**
 * @brief Deinitialize USB clock
 */
void usb_clock_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_USB_H__ */
