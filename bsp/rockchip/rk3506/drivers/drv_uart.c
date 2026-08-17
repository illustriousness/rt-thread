#include <rthw.h>
#include <rtdevice.h>
#include <rtthread.h>

#include <board.h>
#include <drv_uart.h>

#ifdef RT_USING_SERIAL_V1

#ifndef BSP_UART_CLOCK_HZ
#define BSP_UART_CLOCK_HZ 24000000
#endif

#define UART_REG(base, reg)     HWREG32((base) + ((reg) << 2))

#define UART_RBR        0x00
#define UART_THR        0x00
#define UART_DLL        0x00
#define UART_IER        0x01
#define UART_DLH        0x01
#define UART_IIR        0x02
#define UART_FCR        0x02
#define UART_LCR        0x03
#define UART_MCR        0x04
#define UART_LSR        0x05
#define UART_USR        0x1f

#define UART_IER_RX     (1U << 0)
#define UART_IER_TX     (1U << 1)
#define UART_FCR_FIFO   (1U << 0)
#define UART_FCR_RXRST  (1U << 1)
#define UART_FCR_TXRST  (1U << 2)
#define UART_LCR_DLAB   (1U << 7)
#define UART_LCR_8N1    0x03
#define UART_LSR_DR     (1U << 0)
#define UART_LSR_THRE   (1U << 5)
#define UART_USR_BUSY   (1U << 0)

struct rk3506_uart
{
    rt_uint32_t base;
    rt_uint32_t irq;
    const char *name;
    struct rt_serial_device serial;
};

static struct rk3506_uart uart1 =
{
    .base = RK3506_UART1_BASE,
    .irq = RK3506_UART1_IRQ,
    .name = "uart1",
};

#ifdef RT_USING_SMP
static rt_hw_spinlock_t console_lock;
#endif

static struct rk3506_uart *uart_from_serial(struct rt_serial_device *serial)
{
    struct rk3506_uart *uart = serial->parent.user_data;

    /* The low-level console is used before the serial device is registered. */
    return uart ? uart : &uart1;
}

static void uart_wait_idle(struct rk3506_uart *uart)
{
    while (UART_REG(uart->base, UART_USR) & UART_USR_BUSY)
    {
    }
}

static void uart_set_baud(struct rk3506_uart *uart, rt_uint32_t baudrate)
{
    rt_uint32_t divisor;

    if (!baudrate)
    {
        baudrate = BAUD_RATE_115200;
    }

    divisor = BSP_UART_CLOCK_HZ / (16U * baudrate);
    if (!divisor)
    {
        divisor = 1;
    }

    uart_wait_idle(uart);
    UART_REG(uart->base, UART_LCR) = UART_LCR_DLAB;
    UART_REG(uart->base, UART_DLL) = divisor & 0xff;
    UART_REG(uart->base, UART_DLH) = (divisor >> 8) & 0xff;
    UART_REG(uart->base, UART_LCR) = UART_LCR_8N1;
}

static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct rk3506_uart *uart = uart_from_serial(serial);

    uart_set_baud(uart, cfg->baud_rate);
    UART_REG(uart->base, UART_FCR) = UART_FCR_FIFO | UART_FCR_RXRST | UART_FCR_TXRST;
    UART_REG(uart->base, UART_IER) = 0;

    return RT_EOK;
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct rk3506_uart *uart = uart_from_serial(serial);
    rt_ubase_t flag = (rt_ubase_t)arg;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        if (flag == RT_DEVICE_FLAG_INT_RX)
        {
            UART_REG(uart->base, UART_IER) &= ~UART_IER_RX;
        }
        break;

    case RT_DEVICE_CTRL_SET_INT:
        if (flag == RT_DEVICE_FLAG_INT_RX)
        {
            UART_REG(uart->base, UART_IER) |= UART_IER_RX;
            rt_hw_interrupt_umask(uart->irq);
        }
        break;

    default:
        break;
    }

    return RT_EOK;
}

static int uart_putc(struct rt_serial_device *serial, char ch)
{
    struct rk3506_uart *uart = uart_from_serial(serial);

    while (!(UART_REG(uart->base, UART_LSR) & UART_LSR_THRE))
    {
    }

    UART_REG(uart->base, UART_THR) = ch;

    return 1;
}

static int uart_getc(struct rt_serial_device *serial)
{
    struct rk3506_uart *uart = uart_from_serial(serial);

    if (UART_REG(uart->base, UART_LSR) & UART_LSR_DR)
    {
        return UART_REG(uart->base, UART_RBR) & 0xff;
    }

    return -1;
}

static const struct rt_uart_ops uart_ops =
{
    uart_configure,
    uart_control,
    uart_putc,
    uart_getc,
};

static void uart_isr(int vector, void *param)
{
    struct rt_serial_device *serial = param;

    RT_UNUSED(vector);

    while (UART_REG(uart_from_serial(serial)->base, UART_LSR) & UART_LSR_DR)
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
    }
}

void rt_hw_console_output(const char *str)
{
#ifdef RT_USING_SMP
    rt_hw_spin_lock(&console_lock);
#endif

    while (*str)
    {
        if (*str == '\n')
        {
            uart_putc(&uart1.serial, '\r');
        }

        uart_putc(&uart1.serial, *str++);
    }

#ifdef RT_USING_SMP
    rt_hw_spin_unlock(&console_lock);
#endif
}

int rt_hw_uart_init(void)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    config.baud_rate = BAUD_RATE_115200;
    config.bufsz = RT_SERIAL_RB_BUFSZ;

#ifdef RT_USING_SMP
    rt_hw_spin_lock_init(&console_lock);
#endif

    uart1.serial.ops = &uart_ops;
    uart1.serial.config = config;

    uart_configure(&uart1.serial, &config);

    rt_hw_serial_register(&uart1.serial, uart1.name,
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          &uart1);

    rt_hw_interrupt_install(uart1.irq, uart_isr, &uart1.serial, uart1.name);
    rt_hw_interrupt_mask(uart1.irq);

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);

#endif
