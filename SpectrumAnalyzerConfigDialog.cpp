#include "SpectrumAnalyzerConfigDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QFont>

// ======================== 构造函数 ========================
SpectrumAnalyzerConfigDialog::SpectrumAnalyzerConfigDialog(QWidget * parent)
    : QDialog(parent)
{
    setWindowTitle(tr("频谱分析仪参数配置 (Spectrum Analyzer Configuration)"));
    setMinimumWidth(500);
    setMinimumHeight(600);
    setModal(true);
    setupUI();
}

// ======================== setupUI() - 初始化 UI ========================
void SpectrumAnalyzerConfigDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ============ 第一组：参考电平和衰减器 ============
    QGroupBox* group1 = new QGroupBox(tr("参考电平 & 衰减器"), this);
    QGridLayout* group1Layout = new QGridLayout(group1);

    // 参考电平
    refLevelLabel = new QLabel(tr("参考电平 (Reference Level):"), this);
    refLevelSpinBox = new QDoubleSpinBox(this);
    refLevelSpinBox->setRange(-100.0, 20.0);
    refLevelSpinBox->setValue(0.0);
    refLevelSpinBox->setSingleStep(1.0);
    refLevelSpinBox->setDecimals(1);
    refLevelUnitLabel = new QLabel(tr("dBm"), this);
    group1Layout->addWidget(refLevelLabel, 0, 0);
    group1Layout->addWidget(refLevelSpinBox, 0, 1);
    group1Layout->addWidget(refLevelUnitLabel, 0, 2);

    // 衰减器档位
    attenuatorLabel = new QLabel(tr("衰减器档位 (Attenuator):"), this);
    attenuatorSpinBox = new QSpinBox(this);
    attenuatorSpinBox->setRange(0, 70);
    attenuatorSpinBox->setValue(0);
    attenuatorSpinBox->setSingleStep(1);
    attenuatorUnitLabel = new QLabel(tr("dB"), this);
    group1Layout->addWidget(attenuatorLabel, 1, 0);
    group1Layout->addWidget(attenuatorSpinBox, 1, 1);
    group1Layout->addWidget(attenuatorUnitLabel, 1, 2);

    group1Layout->setColumnStretch(1, 1);
    mainLayout->addWidget(group1);

    // ============ 第二组：前置放大 ============
    QGroupBox* group2 = new QGroupBox(tr("前置放大 (Preamplifier)"), this);
    QHBoxLayout* group2Layout = new QHBoxLayout(group2);

    preampLabel = new QLabel(tr("启用前置放大:"), this);
    preampCheckBox = new QCheckBox(tr("启用 (Enable)"), this);
    preampCheckBox->setChecked(false);
    group2Layout->addWidget(preampLabel);
    group2Layout->addWidget(preampCheckBox);
    group2Layout->addStretch();

    mainLayout->addWidget(group2);

    // ============ 第三组：RBW 配置 ============
    QGroupBox* group3 = new QGroupBox(tr("分辨率带宽 (Resolution Bandwidth)"), this);
    QGridLayout* group3Layout = new QGridLayout(group3);

    // RBW 模式
    rbwModeLabel = new QLabel(tr("RBW 模式 (Mode):"), this);
    rbwModeCombo = new QComboBox(this);
    rbwModeCombo->addItem(tr("自适应 (Auto)"), 0);
    rbwModeCombo->addItem(tr("手动 (Manual)"), 1);
    rbwModeCombo->setCurrentIndex(0);
    group3Layout->addWidget(rbwModeLabel, 0, 0);
    group3Layout->addWidget(rbwModeCombo, 0, 1);

    // RBW 值
    rbwValueLabel = new QLabel(tr("RBW 值 (Value):"), this);
    rbwSpinBox = new QDoubleSpinBox(this);
    rbwSpinBox->setRange(1.0, 1e9);
    rbwSpinBox->setValue(10000.0);
    rbwSpinBox->setSingleStep(1000.0);
    rbwSpinBox->setDecimals(0);
    rbwSpinBox->setEnabled(false); // 初始为禁用（自适应模式）
    rbwUnitLabel = new QLabel(tr("Hz"), this);
    group3Layout->addWidget(rbwValueLabel, 1, 0);
    group3Layout->addWidget(rbwSpinBox, 1, 1);
    group3Layout->addWidget(rbwUnitLabel, 1, 2);

    group3Layout->setColumnStretch(1, 1);
    mainLayout->addWidget(group3);

    // ============ 第四组：VBW 配置 ============
    QGroupBox* group4 = new QGroupBox(tr("视频带宽 (Video Bandwidth)"), this);
    QGridLayout* group4Layout = new QGridLayout(group4);

    // VBW 模式
    vbwModeLabel = new QLabel(tr("VBW 模式 (Mode):"), this);
    vbwModeCombo = new QComboBox(this);
    vbwModeCombo->addItem(tr("同 RBW (Same as RBW)"), 0);
    vbwModeCombo->addItem(tr("手动 (Manual)"), 1);
    vbwModeCombo->setCurrentIndex(0);
    group4Layout->addWidget(vbwModeLabel, 0, 0);
    group4Layout->addWidget(vbwModeCombo, 0, 1);

    // VBW 值
    vbwValueLabel = new QLabel(tr("VBW 值 (Value):"), this);
    vbwSpinBox = new QDoubleSpinBox(this);
    vbwSpinBox->setRange(1.0, 1e9);
    vbwSpinBox->setValue(10000.0);
    vbwSpinBox->setSingleStep(1000.0);
    vbwSpinBox->setDecimals(0);
    vbwSpinBox->setEnabled(false); // 初始为禁用（同 RBW 模式）
    vbwUnitLabel = new QLabel(tr("Hz"), this);
    group4Layout->addWidget(vbwValueLabel, 1, 0);
    group4Layout->addWidget(vbwSpinBox, 1, 1);
    group4Layout->addWidget(vbwUnitLabel, 1, 2);

    group4Layout->setColumnStretch(1, 1);
    mainLayout->addWidget(group4);

    // ============ 第五组：耦合方式 ============
    QGroupBox* group5 = new QGroupBox(tr("耦合方式 (Coupling Mode)"), this);
    QHBoxLayout* group5Layout = new QHBoxLayout(group5);

    couplingLabel = new QLabel(tr("选择耦合方式:"), this);
    couplingCombo = new QComboBox(this);
    couplingCombo->addItem("DC");
    couplingCombo->addItem("AC");
    couplingCombo->setCurrentText("DC");
    group5Layout->addWidget(couplingLabel);
    group5Layout->addWidget(couplingCombo);
    group5Layout->addStretch();

    mainLayout->addWidget(group5);

    // ============ 第六组：测量模式 ============
    QGroupBox* group6 = new QGroupBox(tr("测量模式 (Measurement Mode)"), this);
    QHBoxLayout* group6Layout = new QHBoxLayout(group6);

    measurementModeLabel = new QLabel(tr("选择测量模式:"), this);
    measurementModeCombo = new QComboBox(this);
    measurementModeCombo->addItem(tr("扫频 (Sweep)"));
    measurementModeCombo->addItem(tr("IQ (IQ)"));
    measurementModeCombo->setCurrentIndex(0);
    group6Layout->addWidget(measurementModeLabel);
    group6Layout->addWidget(measurementModeCombo);
    group6Layout->addStretch();

    mainLayout->addWidget(group6);

    //// ============ 第七组：幅度和分辨率（新增） ============
    //QGroupBox* group7 = new QGroupBox(tr("幅度 & 分辨率"), this);
    //QGridLayout* group7Layout = new QGridLayout(group7);

    //amplitudeLabel = new QLabel(tr("幅度 (Amplitude):"), this);
    //amplitudeSpinBox = new QDoubleSpinBox(this);
    //amplitudeSpinBox->setRange(-100.0, 20.0);
    //amplitudeSpinBox->setValue(0.0);
    //amplitudeSpinBox->setSingleStep(1.0);
    //amplitudeSpinBox->setDecimals(1);
    //amplitudeUnitLabel = new QLabel(tr("dBm"), this);
    //group7Layout->addWidget(amplitudeLabel, 0, 0);
    //group7Layout->addWidget(amplitudeSpinBox, 0, 1);
    //group7Layout->addWidget(amplitudeUnitLabel, 0, 2);

    //resolutionLabel = new QLabel(tr("分辨率 (Resolution):"), this);
    //resolutionSpinBox = new QDoubleSpinBox(this);
    //resolutionSpinBox->setRange(1.0, 1e9);
    //resolutionSpinBox->setValue(1000.0);
    //resolutionSpinBox->setSingleStep(100.0);
    //resolutionSpinBox->setDecimals(0);
    //resolutionUnitLabel = new QLabel(tr("Hz"), this);
    //group7Layout->addWidget(resolutionLabel, 1, 0);
    //group7Layout->addWidget(resolutionSpinBox, 1, 1);
    //group7Layout->addWidget(resolutionUnitLabel, 1, 2);

    //group7Layout->setColumnStretch(1, 1);
    //mainLayout->addWidget(group7);



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
    connect(rbwModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SpectrumAnalyzerConfigDialog::onRbwModeChanged);
    connect(vbwModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SpectrumAnalyzerConfigDialog::onVbwModeChanged);
    connect(okButton, &QPushButton::clicked, this, &SpectrumAnalyzerConfigDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &SpectrumAnalyzerConfigDialog::onCancelClicked);

    setLayout(mainLayout);
}

// ======================== setConfig() - 设置配置到 UI ========================
void SpectrumAnalyzerConfigDialog::setConfig(const SpectrumAnalyzerConfig& config)
{
    m_config = config;
    loadConfigToUI();
}

// ======================== getConfig() - 获取配置 ========================
SpectrumAnalyzerConfig SpectrumAnalyzerConfigDialog::getConfig() const
{
    return m_config;
}

// ======================== loadConfigToUI() - 加载配置到 UI ========================
void SpectrumAnalyzerConfigDialog::loadConfigToUI()
{
    refLevelSpinBox->setValue(m_config.refLevelDbm);
    attenuatorSpinBox->setValue(m_config.attenuatorDb);
    preampCheckBox->setChecked(m_config.preampEnabled);

    // RBW 配置
    rbwModeCombo->setCurrentIndex(m_config.rbwAutoMode ? 0 : 1);
    rbwSpinBox->setValue(m_config.rbwHz);
    rbwSpinBox->setEnabled(!m_config.rbwAutoMode);

    // VBW 配置
    vbwModeCombo->setCurrentIndex(m_config.vbwSameAsRbw ? 0 : 1);
    vbwSpinBox->setValue(m_config.vbwHz);
    vbwSpinBox->setEnabled(!m_config.vbwSameAsRbw);

    // 耦合和测量模式
    couplingCombo->setCurrentText(m_config.couplingMode);
    int modeIndex = measurementModeCombo->findText(m_config.measurementMode);
    if (modeIndex >= 0) {
        measurementModeCombo->setCurrentIndex(modeIndex);
    }
    //// 幅度和分辨率（新增）
    //amplitudeSpinBox->setValue(m_config.amplitudeDbm);
    //resolutionSpinBox->setValue(m_config.resolutionHz);

}

// ======================== updateConfigFromUI() - 从 UI 更新配置 ========================
void SpectrumAnalyzerConfigDialog::updateConfigFromUI()
{
    m_config.refLevelDbm = refLevelSpinBox->value();
    m_config.attenuatorDb = attenuatorSpinBox->value();
    m_config.preampEnabled = preampCheckBox->isChecked();

    // RBW 配置
    m_config.rbwAutoMode = (rbwModeCombo->currentIndex() == 0);
    m_config.rbwHz = rbwSpinBox->value();

    // VBW 配置
    m_config.vbwSameAsRbw = (vbwModeCombo->currentIndex() == 0);
    m_config.vbwHz = vbwSpinBox->value();

    // 耦合和测量模式
    m_config.couplingMode = couplingCombo->currentText();
    m_config.measurementMode = measurementModeCombo->currentText();
}

// ======================== onRbwModeChanged() ========================
void SpectrumAnalyzerConfigDialog::onRbwModeChanged(int index)
{
    // index 0 = 自适应（禁用输入），index 1 = 手动（启用输入）
    rbwSpinBox->setEnabled(index == 1);
}

// ======================== onVbwModeChanged() ========================
void SpectrumAnalyzerConfigDialog::onVbwModeChanged(int index)
{
    // index 0 = 同 RBW（禁用输入），index 1 = 手动（启用输入）
    vbwSpinBox->setEnabled(index == 1);
}

// ======================== onOkClicked() ========================
void SpectrumAnalyzerConfigDialog::onOkClicked()
{
    updateConfigFromUI();
    accept();
}

// ======================== onCancelClicked() ========================
void SpectrumAnalyzerConfigDialog::onCancelClicked()
{
    reject();
}