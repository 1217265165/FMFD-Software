# VISA TCPIP 连接参考指南

## TCPIP 资源字符串格式

本系统使用 VISA (Virtual Instrument Software Architecture) 标准与仪器通信。以下是 TCPIP 连接的资源字符串格式说明。

### 格式说明

#### 1. INSTR 协议（VXI-11）
```
TCPIP::<IP地址>::inst0::INSTR
```

#### 2. SOCKET 协议（原始 TCP/IP）
```
TCPIP::<IP地址>::<端口号>::SOCKET
```

### 资源字符串示例

| 仪器类型 | 协议 | 资源字符串示例 | 说明 |
|---------|------|---------------|------|
| 信号源 (SG) | INSTR | `TCPIP::192.168.1.200::inst0::INSTR` | 使用 VXI-11 协议，inst0 是默认仪器 |
| 频谱仪 (SA) | SOCKET | `TCPIP::192.168.1.100::5025::SOCKET` | 使用原始 Socket，5025 是 SCPI 标准端口 |
| 频谱仪 (SA) | INSTR | `TCPIP::192.168.1.100::inst0::INSTR` | 也可使用 VXI-11 协议 |

### 两种协议的区别

- **INSTR (VXI-11)：** 标准仪器协议，自动处理超时和错误，推荐用于大多数仪器
- **SOCKET：** 原始 TCP/IP 连接，需要指定端口号（通常是 5025），某些仪器可能需要此协议

### 常用端口号

- **5025：** SCPI 标准 Socket 端口（最常用）
- **111：** VXI-11 端口映射服务
- 其他端口：根据具体仪器厂商文档确定

## SCPI 命令参考

系统使用标准 SCPI (Standard Commands for Programmable Instruments) 命令控制仪器。

### 基本查询命令

| 命令 | 功能 | 示例响应 |
|------|------|---------|
| `*IDN?` | 查询仪器识别信息 | Manufacturer,Model,SerialNumber,FirmwareVersion |
| `*OPC?` | 查询操作完成状态 | 1（表示完成） |
| `*RST` | 复位仪器到默认状态 | 无返回值 |

### 频谱仪 (SA) 常用命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `:INST:CRE "SweptSA","SweptSA0"` | 创建扫频分析实例 | 初始化时使用 |
| `:INST:SEL "SweptSA0"` | 选择扫频分析实例 | 切换到扫频模式 |
| `:FREQ:CENT <freq>` | 设置中心频率（Hz） | `:FREQ:CENT 1E9`（设置 1 GHz） |
| `:FREQ:SPAN <span>` | 设置扫描宽度（Hz） | `:FREQ:SPAN 1E6`（设置 1 MHz） |
| `:BAND:RES <rbw>` | 设置分辨率带宽（Hz） | `:BAND:RES 1000`（设置 1 kHz） |
| `:BAND:VID <vbw>` | 设置视频带宽（Hz） | `:BAND:VID 1000`（设置 1 kHz） |
| `:DISP:WIND:TRAC:Y:SCAL:RLEV <level>` | 设置参考电平（dBm） | `:DISP:WIND:TRAC:Y:SCAL:RLEV 0` |
| `:POW:ATT <atten>` | 设置衰减器（dB） | `:POW:ATT 10`（设置 10 dB） |
| `:POW:GAIN <state>` | 设置前置放大器 | `:POW:GAIN ON` 或 `OFF` |
| `:INP:COUP <mode>` | 设置输入耦合方式 | `:INP:COUP AC` 或 `DC` |
| `:INIT:CONT <state>` | 设置连续/单次扫描 | `:INIT:CONT OFF`（单次扫描） |
| `:INIT` | 触发单次扫描 | 开始一次测量 |
| `:CALC:MARK1:MAX` | 标记 1 移到峰值 | 查找最大值 |
| `:CALC:MARK1:X?` | 查询标记 1 的频率 | 返回频率值（Hz） |
| `:CALC:MARK1:Y?` | 查询标记 1 的幅度 | 返回幅度值（dBm） |

### 信号源 (SG) 常用命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `:FREQ <freq>` | 设置输出频率（Hz） | `:FREQ 1E9`（设置 1 GHz） |
| `:POW <power>` | 设置输出功率（dBm） | `:POW -10`（设置 -10 dBm） |
| `:OUTP <state>` | 开启/关闭输出 | `:OUTP ON` 或 `OFF` |

## 配置步骤

1. 确认仪器的 IP 地址（通过仪器前面板网络设置查看）
2. 确保计算机与仪器在同一网络，或网络可达
3. 在主界面的 SG 和 SA 输入框中填入正确的 VISA 资源字符串
4. 点击开始测量，系统会自动连接仪器
5. 查看输出窗口中的 `*IDN?` 响应，确认连接成功

## 常见问题

### Q: 如何确定使用 SOCKET 还是 INSTR 协议？

A: 根据以下原则选择：
- 查阅仪器编程手册，了解推荐的连接方式
- 如果仪器支持标准 SCPI-over-Socket，使用 SOCKET 协议（端口通常是 5025）
- 如果仪器支持 VXI-11，使用 INSTR 协议（自动处理端口映射）
- 两者都支持时，SOCKET 协议通常更简单直接
- 连接失败时，可尝试切换协议

### Q: 连接仪器时出现错误怎么办？

A: 请检查以下几点：
- 确认仪器电源已开启，网络连接正常
- 检查 IP 地址是否正确（可通过 ping 命令测试连通性）
- 确认 VISA 资源字符串格式正确
- 检查防火墙设置，确保允许相关端口通信
- 尝试使用 NI MAX（Measurement & Automation Explorer）工具测试 VISA 连接

### Q: 如何验证 TCPIP 连接是否正常？

A: 可以按以下步骤验证：

1. **网络连通性测试：**
   ```bash
   ping 192.168.1.100
   ```

2. **端口连通性测试（对于 SOCKET 协议）：**
   ```bash
   telnet 192.168.1.100 5025
   ```

3. **使用 NI MAX 测试 VISA 连接：**
   - 打开 NI MAX 工具
   - 在"设备和接口"中添加 VISA 资源
   - 尝试打开 VISA 会话并发送 `*IDN?` 查询

## 命令使用注意事项

- 所有 SCPI 命令不区分大小写，但建议使用大写以提高可读性
- 命令可以使用简写形式（大写部分）
- 查询命令以 `?` 结尾，设置命令后跟参数值
- 使用 `*OPC?` 等待命令执行完成，避免时序问题
- 不同厂商的仪器可能有细微差异，请参考具体仪器的编程手册

## 支持的仪器型号

本系统主要对接思仪 4082/4052 系列频谱分析仪，遵循标准 SCPI 命令协议。其他兼容 SCPI 的仪器也可通过修改命令实现支持。

---

**注意：** 完整的使用说明可通过软件界面的"帮助 → 使用说明"菜单查看。
