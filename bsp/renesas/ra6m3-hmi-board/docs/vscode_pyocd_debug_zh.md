# RA6M3 HMI Board VSCode + pyOCD 调试说明

本文档对应 `bsp/renesas/ra6m3-hmi-board` 目录下新增的 VSCode 调试配置。

## 1. 目标与适配器

- MCU: `R7FA6M3AH`（target id: `r7fa6m3ah`）
- 调试器: `ART-Link CMSIS-DAP`（USB VID:PID `0416:7687`）
- 调试链路: `VSCode (Cortex-Debug)` + `arm-none-eabi-gdb` + `pyocd gdbserver`

## 2. 依赖安装（Arch Linux）

```bash
sudo pacman -S --needed code arm-none-eabi-gcc arm-none-eabi-gdb arm-none-eabi-newlib
# pyocd 推荐 AUR 包（若 check 失败可加 --mflags "--nocheck"）
yay -S python-pyocd --mflags "--nocheck"
```

可选 USB 权限规则（避免普通用户访问调试器失败）：

```bash
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="0416", ATTR{idProduct}=="7687", TAG+="uaccess"' | \
sudo tee /etc/udev/rules.d/70-art-link-cmsis-dap.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## 3. 新增/整理的文件

- `.vscode/launch.json`
- `.vscode/tasks.json`
- `.vscode/settings.json`
- `.vscode/extensions.json`
- `.vscode/scripts/start-pyocd.sh`
- `.vscode/scripts/stop-pyocd.sh`
- `pyocd.yaml`
- `ra6m3-hmi-board.code-workspace`

## 4. VSCode 调试配置说明

### 4.1 `RA6M3 HMI Debug (pyOCD CMSIS-DAP)`

一键调试主配置，流程如下：

1. 执行 `debug:prepare` 任务
2. `build`（`scons -j$(nproc)`）
3. 自动启动 `pyocd gdbserver`（50000/50001）
4. Cortex-Debug 连接到 `localhost:50000`
5. 结束调试后自动执行 `pyocd:stop`

### 4.2 `RA6M3 HMI Attach (pyOCD CMSIS-DAP)`

由 Cortex-Debug 直接拉起 pyOCD 并 attach，保留为备用配置。

### 4.3 `RA6M3 HMI Attach (external pyOCD :50000)`

用于手动/脚本先启动 server 后再 attach 的场景。

## 5. 手动调试（排查模式）

当需要单独验证 pyOCD 链路时：

```bash
pyocd gdbserver --target r7fa6m3ah --port 50000 --telnet-port 50001
```

然后在 VSCode 里选择 `RA6M3 HMI Attach (external pyOCD :50000)`。

## 6. 常用任务

- `build`
- `clean`
- `rebuild`
- `flash:pyocd`（先 build，再整片擦写+烧录 `rtthread.elf`）

## 7. 日志与进程文件

自动起停脚本会在 `.vscode/` 下生成临时文件：

- `.pyocd.pid`: pyOCD 进程号
- `.pyocd.log`: pyOCD 启动与运行日志

调试结束后 `stop-pyocd.sh` 会清理 `.pyocd.pid` 并终止 server。
