// ======================== SignalGeneratorConfigDialog.cpp ========================
// 信号发生器参数配置对话框 - 实现文件
// 文件编号：[302]

#include "SignalGeneratorConfigDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>

// ======================== 构造函数 ========================
SignalGeneratorConfigDialog::SignalGeneratorConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("信号发生器参数配置 (Signal Generator Configuration)"));
    setMinimumWidth(480);
    setMinimumHeight(480);
    setModal(true);

    setupUI();
}

// ======================== setupUI() - 初始化 UI ========================
void SignalGeneratorConfigDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ============ 第一组：功率配置 ============
    QGroupBox* group1 = new QGroupBox(tr("功率配置 (Power Configuration)"), this);
    QGridLayout* group1Layout = new QGridLayout(group1);

    powerLabel = new QLabel(tr("输出功率 (Output Power):"), this);
    powerSpinBox = new QDoubleSpinBox(this);
    powerSpinBox->setRange(-20.0, 20.0);
    powerSpinBox->setValue(0.0);
    powerSpinBox->setSingleStep(0.1);
    powerSpinBox->setDecimals(1);
    powerUnitLabel = new QLabel(tr("dBm"), this);
    
    group1Layout->addWidget(powerLabel, 0, 0);
    group1Layout->addWidget(powerSpinBox, 0, 1);
    group1Layout->addWidget(powerUnitLabel, 0, 2);
    group1Layout->setColumnStretch(1, 1);
    mainLayout->addWidget(group1);

    // ============ 第二组：补偿设置 ============
    QGroupBox* group2 = new QGroupBox(tr("补偿设置 (Compensation)"), this);
    QVBoxLayout* group2Layout = new QVBoxLayout(group2);

    QHBoxLayout* freqCompLayout = new QHBoxLayout();
    frequencyCompensationLabel = new QLabel(tr("频率补偿 (Frequency Compensation):"), this);
    frequencyCompensationCheckBox = new QCheckBox(tr("启用 (Enable)"), this);
    frequencyCompensationCheckBox->setChecked(false);
    freqCompLayout->addWidget(frequencyCompensationLabel);
    freqCompLayout->addWidget(frequencyCompensationCheckBox);
    freqCompLayout->addStretch();
    group2Layout->addLayout(freqCompLayout);

    QHBoxLayout* powerCompLayout = new QHBoxLayout();
    powerCompensationLabel = new QLabel(tr("功率补偿 (Power Compensation):"), this);
    powerCompensationCheckBox = new QCheckBox(tr("启用 (Enable)"), this);
    powerCompensationCheckBox->setChecked(false);
    powerCompLayout->addWidget(powerCompensationLabel);
    powerCompLayout->addWidget(powerCompensationCheckBox);
    powerCompLayout->addStretch();
    group2Layout->addLayout(powerCompLayout);

    mainLayout->addWidget(group2);

    // ============ 第三组：输出连接器 ============
    QGroupBox* group3 = new QGroupBox(tr("输出连接器 (Output Connector)"), this);
    QHBoxLayout* group3Layout = new QHBoxLayout(group3);

    outputConnectorLabel = new QLabel(tr("连接器类型:"), this);
    outputConnectorCombo = new QComboBox(this);
    outputConnectorCombo->addItem("SMA");
    outputConnectorCombo->addItem("BNC");
    outputConnectorCombo->setCurrentText("SMA");
    group3Layout->addWidget(outputConnectorLabel);
    group3Layout->addWidget(outputConnectorCombo);
    group3Layout->addStretch();

    mainLayout->addWidget(group3);

    // ============ 第四组：功率平坦化 ============
    QGroupBox* group4 = new QGroupBox(tr("功率平坦化 (Power Flattening)"), this);
    QHBoxLayout* group4Layout = new QHBoxLayout(group4);

    powerFlatteningLabel = new QLabel(tr("功率平坦化:"), this);
    powerFlatteningCheckBox = new QCheckBox(tr("启用 (Enable)"), this);
    powerFlatteningCheckBox->setChecked(false);
    group4Layout->addWidget(powerFlatteningLabel);
    group4Layout->addWidget(powerFlatteningCheckBox);
    group4Layout->addStretch();

    mainLayout->addWidget(group4);

    // ============ 第五组：调制模式 ============
    QGroupBox* group5 = new QGroupBox(tr("调制模式 (Modulation Mode)"), this);
    QHBoxLayout* group5Layout = new QHBoxLayout(group5);

    modulationModeLabel = new QLabel(tr("选择调制模式:"), this);
    modulationModeCombo = new QComboBox(this);
    modulationModeCombo->addItem("CW");
    modulationModeCombo->addItem("AM");
    modulationModeCombo->addItem("FM");
    modulationModeCombo->addItem("PM");
    modulationModeCombo->setCurrentText("CW");
    group5Layout->addWidget(modulationModeLabel);
    group5Layout->addWidget(modulationModeCombo);
    group5Layout->addStretch();

    mainLayout->addWidget(group5);

    // ============ 第六组：输出控制 ============
    QGroupBox* group6 = new QGroupBox(tr("输出控制 (Output Control)"), this);
    QHBoxLayout* group6Layout = new QHBoxLayout(group6);

    outputControlLabel = new QLabel(tr("输出状态:"), this);
    outputEnabledCheckBox = new QCheckBox(tr("启用输出 (Enable Output)"), this);
    outputEnabledCheckBox->setChecked(true);
    group6Layout->addWidget(outputControlLabel);
    group6Layout->addWidget(outputEnabledCheckBox);
    group6Layout->addStretch();

    mainLayout->addWidget(group6);

    // ============ 拉伸 ============
    mainLayout->addStretch();

    // ============ 按钮 ============
    QHBoxLayout* btnLayout = new QHBoxLayout();
    okButton = new QPushButton(tr("确定 (OK)"), this);
    cancelButton = new QPushButton(tr("取消 (Cancel)"), this);
    okButton->setMinimumWidth(80);
    cancelButton->setMinimumWidth(80);
    btnLayout->addStretch();
    btnLayout->addWidget(okButton);
    btnLayout->addWidget(cancelButton);
    mainLayout->addLayout(btnLayout);

    // ============ 连接信号槽 ============
    connect(modulationModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SignalGeneratorConfigDialog::onModulationModeChanged);
    connect(outputEnabledCheckBox, &QCheckBox::stateChanged, this, &SignalGeneratorConfigDialog::onOutputEnabledChanged);
    connect(okButton, &QPushButton::clicked, this, &SignalGeneratorConfigDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &SignalGeneratorConfigDialog::onCancelClicked);

    setLayout(mainLayout);
}

// ======================== setConfig() - 设置配置到 UI ========================
void SignalGeneratorConfigDialog::setConfig(const SignalGeneratorConfig& config)
{
    m_config = config;
    loadConfigToUI();
}

// ======================== getConfig() - 获取配置 ========================
SignalGeneratorConfig SignalGeneratorConfigDialog::getConfig() const
{
    return m_config;
}

// ======================== loadConfigToUI() - 加载配置到 UI ========================
void SignalGeneratorConfigDialog::loadConfigToUI()
{
    powerSpinBox->setValue(m_config.powerDbm);
    frequencyCompensationCheckBox->setChecked(m_config.frequencyCompensationEnabled);
    powerCompensationCheckBox->setChecked(m_config.powerCompensationEnabled);
    outputConnectorCombo->setCurrentText(m_config.outputConnector);
    powerFlatteningCheckBox->setChecked(m_config.powerFlatteningEnabled);
    modulationModeCombo->setCurrentText(m_config.modulationMode);
    outputEnabledCheckBox->setChecked(m_config.outputEnabled);
}

// ======================== updateConfigFromUI() - 从 UI 更新配置 ========================
void SignalGeneratorConfigDialog::updateConfigFromUI()
{
    m_config.powerDbm = powerSpinBox->value();
    m_config.frequencyCompensationEnabled = frequencyCompensationCheckBox->isChecked();
    m_config.powerCompensationEnabled = powerCompensationCheckBox->isChecked();
    m_config.outputConnector = outputConnectorCombo->currentText();
    m_config.powerFlatteningEnabled = powerFlatteningCheckBox->isChecked();
    m_config.modulationMode = modulationModeCombo->currentText();
    m_config.outputEnabled = outputEnabledCheckBox->isChecked();
}

// ======================== onModulationModeChanged() ========================
void SignalGeneratorConfigDialog::onModulationModeChanged(int index)
{
    Q_UNUSED(index);
    // 调制模式改变时的处理（目前不做特殊处理，可根据需要扩展）
}

// ======================== onOutputEnabledChanged() ========================
void SignalGeneratorConfigDialog::onOutputEnabledChanged(int state)
{
    Q_UNUSED(state);
    // 输出启用状态改变时的处理（目前不做特殊处理）
}

// ======================== onOkClicked() ========================
void SignalGeneratorConfigDialog::onOkClicked()
{
    updateConfigFromUI();
    accept();
}

// ======================== onCancelClicked() ========================
void SignalGeneratorConfigDialog::onCancelClicked()
{
    reject();
}
