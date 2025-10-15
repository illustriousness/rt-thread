/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-10-14     Claude       The first version
 */

#ifndef __DRV_P3T1755_H__
#define __DRV_P3T1755_H__

#include <rtthread.h>
#include <rtdevice.h>

/* P3T1755 I2C Address options (A0-A2 pins configuration) */
#define P3T1755_I2C_ADDRESS_DEFAULT     (0x48)  /* A2=0, A1=0, A0=0 */
#define P3T1755_I2C_ADDRESS_A0          (0x49)  /* A2=0, A1=0, A0=1 */
#define P3T1755_I2C_ADDRESS_A1          (0x4A)  /* A2=0, A1=1, A0=0 */
#define P3T1755_I2C_ADDRESS_A1_A0       (0x4B)  /* A2=0, A1=1, A0=1 */
#define P3T1755_I2C_ADDRESS_A2          (0x4C)  /* A2=1, A1=0, A0=0 */
#define P3T1755_I2C_ADDRESS_A2_A0       (0x4D)  /* A2=1, A1=0, A0=1 */
#define P3T1755_I2C_ADDRESS_A2_A1       (0x4E)  /* A2=1, A1=1, A0=0 */
#define P3T1755_I2C_ADDRESS_A2_A1_A0    (0x4F)  /* A2=1, A1=1, A0=1 */

/* P3T1755 Register Addresses */
#define P3T1755_REG_TEMP                (0x00)  /* Temperature register (Read-only, 16-bit) */
#define P3T1755_REG_CONF                (0x01)  /* Configuration register (8-bit) */
#define P3T1755_REG_THYST               (0x02)  /* Temperature hysteresis register (16-bit) */
#define P3T1755_REG_TOS                 (0x03)  /* Temperature over-limit register (16-bit) */
#define P3T1755_REG_TIDLE               (0x04)  /* Temperature conversion idle time (8-bit) */
#define P3T1755_REG_TEMP_OFFSET         (0x11)  /* Temperature offset register (16-bit) */

/* Configuration Register Bits */
#define P3T1755_CONF_SD                 (1 << 0)  /* Shutdown mode */
#define P3T1755_CONF_TM                 (1 << 1)  /* Thermostat mode (0=comparator, 1=interrupt) */
#define P3T1755_CONF_POL                (1 << 2)  /* OS polarity (0=active low, 1=active high) */
#define P3T1755_CONF_F0                 (1 << 3)  /* Fault queue bit 0 */
#define P3T1755_CONF_F1                 (1 << 4)  /* Fault queue bit 1 */
#define P3T1755_CONF_R0                 (1 << 5)  /* Converter resolution bit 0 */
#define P3T1755_CONF_R1                 (1 << 6)  /* Converter resolution bit 1 */
#define P3T1755_CONF_OS                 (1 << 7)  /* One-shot conversion trigger */

/* Fault Queue Settings */
#define P3T1755_FAULTQ_1                (0x00)    /* 1 fault */
#define P3T1755_FAULTQ_2                (0x01)    /* 2 consecutive faults */
#define P3T1755_FAULTQ_4                (0x02)    /* 4 consecutive faults */
#define P3T1755_FAULTQ_6                (0x03)    /* 6 consecutive faults */

/* Resolution Settings */
#define P3T1755_RESOLUTION_9BIT         (0x00)    /* 9-bit, conversion time: 27.5ms */
#define P3T1755_RESOLUTION_10BIT        (0x01)    /* 10-bit, conversion time: 55ms */
#define P3T1755_RESOLUTION_11BIT        (0x02)    /* 11-bit, conversion time: 110ms */
#define P3T1755_RESOLUTION_12BIT        (0x03)    /* 12-bit, conversion time: 220ms (default) */

/* Temperature LSB value */
#define P3T1755_TEMP_LSB                (0.0625f) /* Temperature LSB = 0.0625°C */

/* Device structure */
struct p3t1755_device
{
    struct rt_i2c_bus_device *i2c_bus;
    rt_uint8_t i2c_addr;
};

/* Public function prototypes */
struct p3t1755_device *p3t1755_init(const char *i2c_bus_name, rt_uint8_t i2c_addr);
void p3t1755_deinit(struct p3t1755_device *dev);
rt_err_t p3t1755_read_temperature(struct p3t1755_device *dev, float *temp);
rt_err_t p3t1755_set_config(struct p3t1755_device *dev, rt_uint8_t config);
rt_err_t p3t1755_get_config(struct p3t1755_device *dev, rt_uint8_t *config);
rt_err_t p3t1755_set_resolution(struct p3t1755_device *dev, rt_uint8_t resolution);
rt_err_t p3t1755_set_shutdown_mode(struct p3t1755_device *dev, rt_bool_t enable);
rt_err_t p3t1755_set_temp_limit(struct p3t1755_device *dev, float tos, float thyst);
rt_err_t p3t1755_get_temp_limit(struct p3t1755_device *dev, float *tos, float *thyst);
rt_err_t p3t1755_oneshot_conversion(struct p3t1755_device *dev, float *temp);

#endif /* __DRV_P3T1755_H__ */
