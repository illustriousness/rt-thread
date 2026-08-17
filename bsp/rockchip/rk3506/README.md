# RK3506 RT-Thread 移植

该 BSP 从源码构建独立的 RK3506 Cortex-A7 RT-Thread 镜像。第一个里程碑在 f00、
f01、f02 上运行 RT-Thread SMP，使用 UART1，并为每个 CPU 配置一个 ARM
Physical Timer 中断。

编译 ELF、裸二进制和 AMP FIT 镜像：

```sh
make smp-image
```

在源码 U-Boot 中把 `rtthread-smp.img` 下载到未使用地址，不写持久化存储直接启动：

```sh
setenv serverip 192.168.137.1
setenv ipaddr 192.168.137.100
tftpboot 0x0c000000 kernel/bsp/rockchip/rk3506/rtthread-smp.img
ampboot 0x0c000000
```

SMP 成功启动后，每个绑定 CPU 每秒输出一行 `SMP`，且各 CPU 的 `timer_irqs`
必须持续增加。

为物理核心 f01 编译单核 AMP 镜像：

```sh
make amp-f01-image
```

AMP 镜像链接地址为 `0x05000000`，只使用 1 MiB RAM，并将物理 f01 映射为
RT-Thread 逻辑 CPU0。从 U-Boot 启动：

```sh
tftpboot 0x0c000000 kernel/bsp/rockchip/rk3506/rtthread-amp-f01.img
ampboot 0x0c000000
```

AMP 成功启动后，U-Boot 继续在 f00 运行，f01 每秒输出一行 `AMP`，其中
`timer_irqs` 持续增加。
