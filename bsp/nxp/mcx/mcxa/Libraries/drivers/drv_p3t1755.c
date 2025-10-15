/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-10-14     Claude       The first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_p3t1755.h"

#define DBG_TAG    "drv.p3t1755"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

/**
 * @brief Write data to P3T1755 register
 *
 * @param dev Device structure
 * @param reg Register address
 * @param data Data buffer
 * @param len Data length
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
static rt_err_t p3t1755_write_reg(struct p3t1755_device *dev, rt_uint8_t reg, rt_uint8_t *data, rt_uint8_t len)
{
    struct rt_i2c_msg msgs;
    rt_uint8_t buf[16];

    if (len > 15)
    {
        LOG_E("Write data too long");
        return -RT_ERROR;
    }

    buf[0] = reg;
    rt_memcpy(&buf[1], data, len);

    msgs.addr = dev->i2c_addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = len + 1;

    if (rt_i2c_transfer(dev->i2c_bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        LOG_E("Write register 0x%02x failed", reg);
        return -RT_ERROR;
    }
}

/**
 * @brief Read data from P3T1755 register
 *
 * @param dev Device structure
 * @param reg Register address
 * @param data Data buffer
 * @param len Data length
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
static rt_err_t p3t1755_read_reg(struct p3t1755_device *dev, rt_uint8_t reg, rt_uint8_t *data, rt_uint8_t len)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr = dev->i2c_addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &reg;
    msgs[0].len = 1;

    msgs[1].addr = dev->i2c_addr;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = data;
    msgs[1].len = len;

    if (rt_i2c_transfer(dev->i2c_bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
        LOG_E("Read register 0x%02x failed", reg);
        return -RT_ERROR;
    }
}

/**
 * @brief Initialize P3T1755 device
 *
 * @param i2c_bus_name I2C bus name
 * @param i2c_addr I2C address
 * @return struct p3t1755_device* Device structure pointer, RT_NULL on failure
 */
struct p3t1755_device *p3t1755_init(const char *i2c_bus_name, rt_uint8_t i2c_addr)
{
    struct p3t1755_device *dev = RT_NULL;

    RT_ASSERT(i2c_bus_name);

    dev = rt_malloc(sizeof(struct p3t1755_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for P3T1755 device");
        return RT_NULL;
    }

    dev->i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(i2c_bus_name);
    if (dev->i2c_bus == RT_NULL)
    {
        LOG_E("Can't find %s device", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->i2c_addr = i2c_addr;

    LOG_I("P3T1755 initialized on %s, address 0x%02x", i2c_bus_name, i2c_addr);

    return dev;
}

/**
 * @brief Deinitialize P3T1755 device
 *
 * @param dev Device structure
 */
void p3t1755_deinit(struct p3t1755_device *dev)
{
    RT_ASSERT(dev);

    rt_free(dev);
}

/**
 * @brief Read temperature from P3T1755
 *
 * @param dev Device structure
 * @param temp Temperature value in Celsius
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
rt_err_t p3t1755_read_temperature(struct p3t1755_device *dev, float *temp)
{
    rt_uint8_t data[2];
    rt_int16_t raw_temp;
    rt_err_t result;

    RT_ASSERT(dev);
    RT_ASSERT(temp);

    result = p3t1755_read_reg(dev, P3T1755_REG_TEMP, data, 2);
    if (result != RT_EOK)
    {
        return result;
    }

    /* Combine MSB and LSB */
    raw_temp = (rt_int16_t)((data[0] << 8) | data[1]);

    /* Convert to temperature (12-bit resolution, LSB = 0.0625°C) */
    /* Temperature data is in bits [15:4], sign-extended */
    raw_temp = raw_temp >> 4;
    *temp = (float)raw_temp * P3T1755_TEMP_LSB;

    return RT_EOK;
}

/**
 * @brief Set P3T1755 configuration register
 *
 * @param dev Device structure
 * @param config Configuration value
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
rt_err_t p3t1755_set_config(struct p3t1755_device *dev, rt_uint8_t config)
{
    RT_ASSERT(dev);

    return p3t1755_write_reg(dev, P3T1755_REG_CONF, &config, 1);
}

/**
 * @brief Get P3T1755 configuration register
 *
 * @param dev Device structure
 * @param config Configuration value
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
rt_err_t p3t1755_get_config(struct p3t1755_device *dev, rt_uint8_t *config)
{
    RT_ASSERT(dev);
    RT_ASSERT(config);

    return p3t1755_read_reg(dev, P3T1755_REG_CONF, config, 1);
}

/**
 * @brief Set P3T1755 resolution
 *
 * @param dev Device structure
 * @param resolution Resolution setting (P3T1755_RESOLUTION_9BIT to P3T1755_RESOLUTION_12BIT)
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
rt_err_t p3t1755_set_resolution(struct p3t1755_device *dev, rt_uint8_t resolution)
{
    rt_uint8_t config;
    rt_err_t result;

    RT_ASSERT(dev);

    if (resolution > P3T1755_RESOLUTION_12BIT)
    {
        LOG_E("Invalid resolution value");
        return -RT_EINVAL;
    }

    result = p3t1755_get_config(dev, &config);
    if (result != RT_EOK)
    {
        return result;
    }

    /* Clear resolution bits and set new value */
    config &= ~(P3T1755_CONF_R0 | P3T1755_CONF_R1);
    config |= (resolution << 5);

    return p3t1755_set_config(dev, config);
}

/**
 * @brief Set P3T1755 shutdown mode
 *
 * @param dev Device structure
 * @param enable RT_TRUE to enable shutdown mode, RT_FALSE to disable
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
rt_err_t p3t1755_set_shutdown_mode(struct p3t1755_device *dev, rt_bool_t enable)
{
    rt_uint8_t config;
    rt_err_t result;

    RT_ASSERT(dev);

    result = p3t1755_get_config(dev, &config);
    if (result != RT_EOK)
    {
        return result;
    }

    if (enable)
    {
        config |= P3T1755_CONF_SD;
    }
    else
    {
        config &= ~P3T1755_CONF_SD;
    }

    return p3t1755_set_config(dev, config);
}

/**
 * @brief Set P3T1755 temperature limits
 *
 * @param dev Device structure
 * @param tos Over-temperature limit in Celsius
 * @param thyst Hysteresis temperature in Celsius
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
rt_err_t p3t1755_set_temp_limit(struct p3t1755_device *dev, float tos, float thyst)
{
    rt_uint8_t data[2];
    rt_int16_t raw_temp;
    rt_err_t result;

    RT_ASSERT(dev);

    /* Set TOS register */
    raw_temp = (rt_int16_t)(tos / P3T1755_TEMP_LSB);
    raw_temp = raw_temp << 4;  /* Shift to bits [15:4] */
    data[0] = (rt_uint8_t)(raw_temp >> 8);
    data[1] = (rt_uint8_t)(raw_temp & 0xFF);

    result = p3t1755_write_reg(dev, P3T1755_REG_TOS, data, 2);
    if (result != RT_EOK)
    {
        return result;
    }

    /* Set THYST register */
    raw_temp = (rt_int16_t)(thyst / P3T1755_TEMP_LSB);
    raw_temp = raw_temp << 4;  /* Shift to bits [15:4] */
    data[0] = (rt_uint8_t)(raw_temp >> 8);
    data[1] = (rt_uint8_t)(raw_temp & 0xFF);

    return p3t1755_write_reg(dev, P3T1755_REG_THYST, data, 2);
}

/**
 * @brief Get P3T1755 temperature limits
 *
 * @param dev Device structure
 * @param tos Over-temperature limit in Celsius
 * @param thyst Hysteresis temperature in Celsius
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
rt_err_t p3t1755_get_temp_limit(struct p3t1755_device *dev, float *tos, float *thyst)
{
    rt_uint8_t data[2];
    rt_int16_t raw_temp;
    rt_err_t result;

    RT_ASSERT(dev);
    RT_ASSERT(tos);
    RT_ASSERT(thyst);

    /* Read TOS register */
    result = p3t1755_read_reg(dev, P3T1755_REG_TOS, data, 2);
    if (result != RT_EOK)
    {
        return result;
    }

    raw_temp = (rt_int16_t)((data[0] << 8) | data[1]);
    raw_temp = raw_temp >> 4;
    *tos = (float)raw_temp * P3T1755_TEMP_LSB;

    /* Read THYST register */
    result = p3t1755_read_reg(dev, P3T1755_REG_THYST, data, 2);
    if (result != RT_EOK)
    {
        return result;
    }

    raw_temp = (rt_int16_t)((data[0] << 8) | data[1]);
    raw_temp = raw_temp >> 4;
    *thyst = (float)raw_temp * P3T1755_TEMP_LSB;

    return RT_EOK;
}

/**
 * @brief Perform one-shot temperature conversion
 *
 * @param dev Device structure
 * @param temp Temperature value in Celsius
 * @return rt_err_t RT_EOK on success, error code otherwise
 */
rt_err_t p3t1755_oneshot_conversion(struct p3t1755_device *dev, float *temp)
{
    rt_uint8_t config;
    rt_err_t result;

    RT_ASSERT(dev);
    RT_ASSERT(temp);

    /* Enable shutdown mode if not already enabled */
    result = p3t1755_get_config(dev, &config);
    if (result != RT_EOK)
    {
        return result;
    }

    if (!(config & P3T1755_CONF_SD))
    {
        config |= P3T1755_CONF_SD;
        result = p3t1755_set_config(dev, config);
        if (result != RT_EOK)
        {
            return result;
        }
    }

    /* Trigger one-shot conversion */
    config |= P3T1755_CONF_OS;
    result = p3t1755_set_config(dev, config);
    if (result != RT_EOK)
    {
        return result;
    }

    /* Wait for conversion to complete (max 220ms for 12-bit) */
    rt_thread_mdelay(250);

    /* Read temperature */
    return p3t1755_read_temperature(dev, temp);
}

#ifdef RT_USING_FINSH
#include <finsh.h>

static struct p3t1755_device *p3t1755_test_dev = RT_NULL;

/**
 * @brief Test P3T1755 sensor - Initialize
 *
 * @param i2c_bus I2C bus name
 * @param addr I2C address (in hex, e.g., 0x48)
 */
static void p3t1755_probe(int argc, char **argv)
{
    const char *i2c_bus = "i2c0";
    rt_uint8_t addr = P3T1755_I2C_ADDRESS_DEFAULT;

    if (argc >= 2)
    {
        i2c_bus = argv[1];
    }

    if (argc >= 3)
    {
        addr = (rt_uint8_t)strtol(argv[2], RT_NULL, 16);
    }

    if (p3t1755_test_dev != RT_NULL)
    {
        rt_kprintf("P3T1755 already initialized, deinitializing first...\n");
        p3t1755_deinit(p3t1755_test_dev);
    }

    p3t1755_test_dev = p3t1755_init(i2c_bus, addr);
    if (p3t1755_test_dev == RT_NULL)
    {
        rt_kprintf("Failed to initialize P3T1755\n");
    }
    else
    {
        rt_kprintf("P3T1755 initialized successfully on %s at 0x%02X\n", i2c_bus, addr);
    }
}
MSH_CMD_EXPORT(p3t1755_probe, Probe P3T1755 sensor: p3t1755_probe [i2c_bus] [addr]);

/**
 * @brief Test P3T1755 sensor - Read temperature
 */
static void p3t1755_temp(void)
{
    float temp;
    rt_err_t result;

    if (p3t1755_test_dev == RT_NULL)
    {
        rt_kprintf("P3T1755 not initialized, run p3t1755_probe first\n");
        return;
    }

    result = p3t1755_read_temperature(p3t1755_test_dev, &temp);
    if (result == RT_EOK)
    {
        rt_kprintf("Temperature: %.2f C\n", temp);
    }
    else
    {
        rt_kprintf("Failed to read temperature\n");
    }
}
MSH_CMD_EXPORT(p3t1755_temp, Read temperature from P3T1755);

/**
 * @brief Test P3T1755 sensor - Set resolution
 *
 * @param resolution Resolution (0=9bit, 1=10bit, 2=11bit, 3=12bit)
 */
static void p3t1755_res(int argc, char **argv)
{
    rt_uint8_t resolution;
    rt_err_t result;

    if (p3t1755_test_dev == RT_NULL)
    {
        rt_kprintf("P3T1755 not initialized, run p3t1755_probe first\n");
        return;
    }

    if (argc < 2)
    {
        rt_kprintf("Usage: p3t1755_res <0-3> (0=9bit, 1=10bit, 2=11bit, 3=12bit)\n");
        return;
    }

    resolution = (rt_uint8_t)atoi(argv[1]);
    result = p3t1755_set_resolution(p3t1755_test_dev, resolution);
    if (result == RT_EOK)
    {
        rt_kprintf("Resolution set to %d-bit\n", 9 + resolution);
    }
    else
    {
        rt_kprintf("Failed to set resolution\n");
    }
}
MSH_CMD_EXPORT(p3t1755_res, Set P3T1755 resolution: p3t1755_res <0-3>);

/**
 * @brief Test P3T1755 sensor - Read configuration
 */
static void p3t1755_conf(void)
{
    rt_uint8_t config;
    rt_err_t result;

    if (p3t1755_test_dev == RT_NULL)
    {
        rt_kprintf("P3T1755 not initialized, run p3t1755_probe first\n");
        return;
    }

    result = p3t1755_get_config(p3t1755_test_dev, &config);
    if (result == RT_EOK)
    {
        rt_kprintf("Configuration: 0x%02X\n", config);
        rt_kprintf("  Shutdown: %s\n", (config & P3T1755_CONF_SD) ? "Yes" : "No");
        rt_kprintf("  Mode: %s\n", (config & P3T1755_CONF_TM) ? "Interrupt" : "Comparator");
        rt_kprintf("  Polarity: %s\n", (config & P3T1755_CONF_POL) ? "Active High" : "Active Low");
        rt_kprintf("  Resolution: %d-bit\n", 9 + ((config >> 5) & 0x03));
    }
    else
    {
        rt_kprintf("Failed to read configuration\n");
    }
}
MSH_CMD_EXPORT(p3t1755_conf, Read P3T1755 configuration);

#endif /* RT_USING_FINSH */
