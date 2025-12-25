#include "UsageDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

UsageDialog::UsageDialog(QWidget* parent)
    : QDialog(parent)
{
    initializeUI();
}

UsageDialog::~UsageDialog()
{
}

void UsageDialog::initializeUI()
{
    setWindowTitle("使用说明");
    setGeometry(100, 100, 700, 600);
    setModal(true);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 标题
    QLabel* titleLabel = new QLabel("频谱分析仪测量故障诊断系统 - 使用说明");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // 滚动区域和文本显示
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);

    m_textEdit = new QTextEdit;
    m_textEdit->setReadOnly(true);
    m_textEdit->setHtml(getUsageText());
    scrollArea->setWidget(m_textEdit);
    mainLayout->addWidget(scrollArea);

    // 关闭按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    m_closeButton = new QPushButton("关闭");
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(m_closeButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

QString UsageDialog::getUsageText()
{
    return QString(
        "<h2>一、系统概述</h2>"
        "<p>本系统用于频谱分析仪的测量和故障诊断，通过配置扫频参数、频谱仪参数和信号源参数进行多频点测量，采集信号特征数据用于故障诊断。</p>"

        "<h2>二、主要功能模块</h2>"
        "<h3>1. 扫频配置</h3>"
        "<ul>"
        "<li><b>频率范围设置：</b>可设置多个扫频段，每段包含起始频率、停止频率和步长</li>"
        "<li><b>模式选择：</b>支持分段生成和文件导入两种模式</li>"
        "<li><b>默认配置：</b>提供了 10kHz~67GHz 的四段默认扫频计划</li>"
        "</ul>"

        "<h3>2. 频谱分析仪参数配置</h3>"
        "<ul>"
        "<li><b>参考电平 (Ref Level)：</b>设置测量的基准电平，通常为 0 dBm</li>"
        "<li><b>RBW (分辨率带宽)：</b>频谱分辨率，数值越小分辨率越高，测量时间越长</li>"
        "<li><b>VBW (视频带宽)：</b>视频滤波带宽，通常与 RBW 相同</li>"
        "<li><b>前放使能：</b>低信号时启用，增强灵敏度</li>"
        "<li><b>衰减器：</b>防止强信号过载</li>"
        "</ul>"

        "<h3>3. 信号源参数配置</h3>"
        "<ul>"
        "<li><b>功率设置：</b>设置输出信号功率（单位 dBm）</li>"
        "<li><b>输出开关：</b>启用/禁用信号输出</li>"
        "<li><b>补偿设置：</b>频率和功率补偿选项</li>"
        "</ul>"

        "<h3>4. 方案管理</h3>"
        "<ul>"
        "<li><b>保存方案：</b>将当前测量参数保存为方案，便于重复使用</li>"
        "<li><b>加载方案：</b>快速加载已保存的方案参数</li>"
        "<li><b>删除方案：</b>移除不需要的方案</li>"
        "</ul>"

        "<h2>三、VISA 资源字符串配置（TCPIP 连接）</h2>"
        "<p>本系统使用 VISA (Virtual Instrument Software Architecture) 标准与仪器通信。TCPIP 是最常用的连接方式。</p>"
        
        "<h3>1. TCPIP 资源字符串格式</h3>"
        "<p><b>格式说明：</b></p>"
        "<ul>"
        "<li><b>INSTR 协议（VXI-11）：</b><code>TCPIP::&lt;IP地址&gt;::inst0::INSTR</code></li>"
        "<li><b>SOCKET 协议（原始 TCP/IP）：</b><code>TCPIP::&lt;IP地址&gt;::&lt;端口号&gt;::SOCKET</code></li>"
        "</ul>"
        
        "<h3>2. 资源字符串示例</h3>"
        "<table border='1' cellpadding='5' cellspacing='0' style='border-collapse: collapse; width: 100%;'>"
        "<tr style='background-color: #f0f0f0;'>"
        "<th>仪器类型</th><th>协议</th><th>资源字符串示例</th><th>说明</th>"
        "</tr>"
        "<tr>"
        "<td>信号源 (SG)</td>"
        "<td>INSTR</td>"
        "<td><code>TCPIP::192.168.1.200::inst0::INSTR</code></td>"
        "<td>使用 VXI-11 协议，inst0 是默认仪器</td>"
        "</tr>"
        "<tr>"
        "<td>频谱仪 (SA)</td>"
        "<td>SOCKET</td>"
        "<td><code>TCPIP::192.168.1.100::5025::SOCKET</code></td>"
        "<td>使用原始 Socket，5025 是 SCPI 标准端口</td>"
        "</tr>"
        "<tr>"
        "<td>频谱仪 (SA)</td>"
        "<td>INSTR</td>"
        "<td><code>TCPIP::192.168.1.100::inst0::INSTR</code></td>"
        "<td>也可使用 VXI-11 协议</td>"
        "</tr>"
        "</table>"
        
        "<h3>3. 两种协议的区别</h3>"
        "<ul>"
        "<li><b>INSTR (VXI-11)：</b>标准仪器协议，自动处理超时和错误，推荐用于大多数仪器</li>"
        "<li><b>SOCKET：</b>原始 TCP/IP 连接，需要指定端口号（通常是 5025），某些仪器可能需要此协议</li>"
        "</ul>"
        
        "<h3>4. 配置步骤</h3>"
        "<ol>"
        "<li>确认仪器的 IP 地址（通过仪器前面板网络设置查看）</li>"
        "<li>确保计算机与仪器在同一网络，或网络可达</li>"
        "<li>在主界面的 SG 和 SA 输入框中填入正确的 VISA 资源字符串</li>"
        "<li>点击开始测量，系统会自动连接仪器</li>"
        "<li>查看输出窗口中的 *IDN? 响应，确认连接成功</li>"
        "</ol>"
        
        "<h3>5. 常用端口号</h3>"
        "<ul>"
        "<li><b>5025：</b>SCPI 标准 Socket 端口（最常用）</li>"
        "<li><b>111：</b>VXI-11 端口映射服务</li>"
        "<li>其他端口：根据具体仪器厂商文档确定</li>"
        "</ul>"
        
        "<h2>四、SCPI 命令参考</h2>"
        "<p>系统使用标准 SCPI (Standard Commands for Programmable Instruments) 命令控制仪器。</p>"
        
        "<h3>1. 基本查询命令</h3>"
        "<table border='1' cellpadding='5' cellspacing='0' style='border-collapse: collapse; width: 100%;'>"
        "<tr style='background-color: #f0f0f0;'>"
        "<th>命令</th><th>功能</th><th>示例响应</th>"
        "</tr>"
        "<tr>"
        "<td><code>*IDN?</code></td>"
        "<td>查询仪器识别信息</td>"
        "<td>Manufacturer,Model,SerialNumber,FirmwareVersion</td>"
        "</tr>"
        "<tr>"
        "<td><code>*OPC?</code></td>"
        "<td>查询操作完成状态</td>"
        "<td>1（表示完成）</td>"
        "</tr>"
        "<tr>"
        "<td><code>*RST</code></td>"
        "<td>复位仪器到默认状态</td>"
        "<td>无返回值</td>"
        "</tr>"
        "</table>"
        
        "<h3>2. 频谱仪 (SA) 命令</h3>"
        "<table border='1' cellpadding='5' cellspacing='0' style='border-collapse: collapse; width: 100%;'>"
        "<tr style='background-color: #f0f0f0;'>"
        "<th>命令</th><th>功能</th><th>示例</th>"
        "</tr>"
        "<tr>"
        "<td><code>:INST:CRE \"SweptSA\",\"SweptSA0\"</code></td>"
        "<td>创建扫频分析实例</td>"
        "<td>初始化时使用</td>"
        "</tr>"
        "<tr>"
        "<td><code>:INST:SEL \"SweptSA0\"</code></td>"
        "<td>选择扫频分析实例</td>"
        "<td>切换到扫频模式</td>"
        "</tr>"
        "<tr>"
        "<td><code>:FREQ:CENT &lt;freq&gt;</code></td>"
        "<td>设置中心频率（Hz）</td>"
        "<td>:FREQ:CENT 1E9（设置 1 GHz）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:FREQ:SPAN &lt;span&gt;</code></td>"
        "<td>设置扫描宽度（Hz）</td>"
        "<td>:FREQ:SPAN 1E6（设置 1 MHz）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:BAND:RES &lt;rbw&gt;</code></td>"
        "<td>设置分辨率带宽（Hz）</td>"
        "<td>:BAND:RES 1000（设置 1 kHz）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:BAND:VID &lt;vbw&gt;</code></td>"
        "<td>设置视频带宽（Hz）</td>"
        "<td>:BAND:VID 1000（设置 1 kHz）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:DISP:WIND:TRAC:Y:SCAL:RLEV &lt;level&gt;</code></td>"
        "<td>设置参考电平（dBm）</td>"
        "<td>:DISP:WIND:TRAC:Y:SCAL:RLEV 0（设置 0 dBm）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:POW:ATT &lt;atten&gt;</code></td>"
        "<td>设置衰减器（dB）</td>"
        "<td>:POW:ATT 10（设置 10 dB）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:POW:GAIN &lt;state&gt;</code></td>"
        "<td>设置前置放大器</td>"
        "<td>:POW:GAIN ON 或 OFF</td>"
        "</tr>"
        "<tr>"
        "<td><code>:INP:COUP &lt;mode&gt;</code></td>"
        "<td>设置输入耦合方式</td>"
        "<td>:INP:COUP AC 或 DC</td>"
        "</tr>"
        "<tr>"
        "<td><code>:INIT:CONT &lt;state&gt;</code></td>"
        "<td>设置连续/单次扫描</td>"
        "<td>:INIT:CONT OFF（单次扫描）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:INIT</code></td>"
        "<td>触发单次扫描</td>"
        "<td>开始一次测量</td>"
        "</tr>"
        "<tr>"
        "<td><code>:CALC:MARK1:MAX</code></td>"
        "<td>标记 1 移到峰值</td>"
        "<td>查找最大值</td>"
        "</tr>"
        "<tr>"
        "<td><code>:CALC:MARK1:X?</code></td>"
        "<td>查询标记 1 的频率</td>"
        "<td>返回频率值（Hz）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:CALC:MARK1:Y?</code></td>"
        "<td>查询标记 1 的幅度</td>"
        "<td>返回幅度值（dBm）</td>"
        "</tr>"
        "</table>"
        
        "<h3>3. 信号源 (SG) 命令</h3>"
        "<table border='1' cellpadding='5' cellspacing='0' style='border-collapse: collapse; width: 100%;'>"
        "<tr style='background-color: #f0f0f0;'>"
        "<th>命令</th><th>功能</th><th>示例</th>"
        "</tr>"
        "<tr>"
        "<td><code>:FREQ &lt;freq&gt;</code></td>"
        "<td>设置输出频率（Hz）</td>"
        "<td>:FREQ 1E9（设置 1 GHz）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:POW &lt;power&gt;</code></td>"
        "<td>设置输出功率（dBm）</td>"
        "<td>:POW -10（设置 -10 dBm）</td>"
        "</tr>"
        "<tr>"
        "<td><code>:OUTP &lt;state&gt;</code></td>"
        "<td>开启/关闭输出</td>"
        "<td>:OUTP ON 或 OFF</td>"
        "</tr>"
        "</table>"
        
        "<h3>4. 命令使用注意事项</h3>"
        "<ul>"
        "<li>所有 SCPI 命令不区分大小写，但建议使用大写以提高可读性</li>"
        "<li>命令可以使用简写形式（大写部分），如 <code>:FREQuency:CENTer</code> 可简写为 <code>:FREQ:CENT</code></li>"
        "<li>查询命令以 <code>?</code> 结尾，设置命令后跟参数值</li>"
        "<li>使用 <code>*OPC?</code> 等待命令执行完成，避免时序问题</li>"
        "<li>不同厂商的仪器可能有细微差异，请参考具体仪器的编程手册</li>"
        "</ul>"
        
        "<h2>五、测量流程</h2>"
        "<ol>"
        "<li><b>连接仪器：</b>确保频谱仪和信号源已通过 VISA 正确连接，填写正确的 TCPIP 资源字符串</li>"
        "<li><b>配置参数：</b>在相应对话框中设置扫频范围、SA 参数和 SG 参数</li>"
        "<li><b>保存方案（可选）：</b>点击方案管理保存当前配置</li>"
        "<li><b>开始测量：</b>点击「一键开始」按钮启动测量</li>"
        "<li><b>查看结果：</b>在结果表中查看每个频点的测量数据</li>"
        "<li><b>导出数据：</b>选择要导出的数据行，点击导出按钮保存为 CSV/Excel</li>"
        "</ol>"

        "<h2>六、输出窗口说明</h2>"
        "<p>点击菜单 <b>调试 → 窗口 → 输出</b> (快捷键 Ctrl+Alt+O) 查看实时日志：</p>"
        "<ul>"
        "<li><b>[SIM]：</b>模拟模式下的输出信息</li>"
        "<li><b>SA *IDN?：</b>频谱仪的型号识别信息，表示与 SA 连接成功</li>"
        "<li><b>SG *IDN?：</b>信号源的型号识别信息，表示与 SG 连接成功</li>"
        "<li>若无这些信息，说明未进行真实仪器连接（可能仍在模拟模式）</li>"
        "</ul>"

        "<h2>七、常见问题</h2>"
        "<h3>Q: TCPIP 资源字符串应该使用什么格式？</h3>"
        "<p>A: 根据仪器支持的协议选择：</p>"
        "<ul>"
        "<li><b>VXI-11 协议：</b>使用格式 <code>TCPIP::IP地址::inst0::INSTR</code>，例如 <code>TCPIP::192.168.1.100::inst0::INSTR</code></li>"
        "<li><b>Socket 协议：</b>使用格式 <code>TCPIP::IP地址::端口::SOCKET</code>，例如 <code>TCPIP::192.168.1.100::5025::SOCKET</code></li>"
        "<li>思仪 4082/4052 系列两种协议都支持，优先尝试 SOCKET 协议（端口 5025）</li>"
        "</ul>"
        
        "<h3>Q: 如何确定使用 SOCKET 还是 INSTR 协议？</h3>"
        "<p>A: 根据以下原则选择：</p>"
        "<ul>"
        "<li>查阅仪器编程手册，了解推荐的连接方式</li>"
        "<li>如果仪器支持标准 SCPI-over-Socket，使用 SOCKET 协议（端口通常是 5025）</li>"
        "<li>如果仪器支持 VXI-11，使用 INSTR 协议（自动处理端口映射）</li>"
        "<li>两者都支持时，SOCKET 协议通常更简单直接</li>"
        "<li>连接失败时，可尝试切换协议</li>"
        "</ul>"
        
        "<h3>Q: 如何启用真实仪器控制？</h3>"
        "<p>A: 修改 InstrumentController.cpp 中的 #define SIMULATION_MODE 为 0，重新编译，并确保 NI-VISA 已安装并正确配置 VISA 资源名。</p>"

        "<h3>Q: 测量时间太长怎么办？</h3>"
        "<p>A: 可以增大 RBW 值（分辨率带宽）或减少扫频段数来加快测量速度，但会牺牲频率分辨率。</p>"

        "<h3>Q: 如何确认与仪器的连接？</h3>"
        "<p>A: 打开输出窗口，查看是否能看到仪器的 *IDN? 查询结果。如果看到仪器型号，表示连接成功。</p>"

        "<h3>Q: 支持哪些频谱仪型号？</h3>"
        "<p>A: 系统对接思仪 4082/4052 系列，遵循标准 SCPI 命令协议，其他兼容 SCPI 的仪器也可通过修改命令实现支持。</p>"
        
        "<h3>Q: 连接仪器时出现错误怎么办？</h3>"
        "<p>A: 请检查以下几点：</p>"
        "<ul>"
        "<li>确认仪器电源已开启，网络连接正常</li>"
        "<li>检查 IP 地址是否正确（可通过 ping 命令测试连通性）</li>"
        "<li>确认 VISA 资源字符串格式正确</li>"
        "<li>检查防火墙设置，确保允许相关端口通信</li>"
        "<li>尝试使用 NI MAX（Measurement & Automation Explorer）工具测试 VISA 连接</li>"
        "</ul>"

        "<h2>八、技术支持</h2>"
        "<p>如有问题或建议，请联系系统开发者或查阅相关文档。</p>"
    );
}