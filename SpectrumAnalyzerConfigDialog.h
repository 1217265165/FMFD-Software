
#ifndef SPECTRUMANALYZERCONFIGDIALOG_H
#define SPECTRUMANALYZERCONFIGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include "CommonTypes.h"

// 频谱分析仪配置对话框类
class SpectrumAnalyzerConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpectrumAnalyzerConfigDialog(QWidget* parent = nullptr);
    ~SpectrumAnalyzerConfigDialog() = default;

    // 设置配置数据到对话框
    void setConfig(const SpectrumAnalyzerConfig& config);

    // 从对话框获取配置数据
    SpectrumAnalyzerConfig getConfig() const;

private:
    // 初始化 UI
    void setupUI();

    // 从对话框更新配置数据
    void updateConfigFromUI();

    // 加载配置数据到对话框
    void loadConfigToUI();

    // 槽函数 - RBW 模式变化
private slots:
    void onRbwModeChanged(int index);

    // 槽函数 - VBW 模式变化
private slots:
    void onVbwModeChanged(int index);

    // 槽函数 - 确定按钮
private slots:
    void onOkClicked();

    // 槽函数 - 取消按钮
private slots:
    void onCancelClicked();

private:
    // ============ UI 控件 ============

    // 第一组：参考电平和衰减器
    QLabel* refLevelLabel = nullptr;
    QDoubleSpinBox* refLevelSpinBox = nullptr;
    QLabel* refLevelUnitLabel = nullptr;

    QLabel* attenuatorLabel = nullptr;
    QSpinBox* attenuatorSpinBox = nullptr;
    QLabel* attenuatorUnitLabel = nullptr;

    // 第二组：前置放大
    QLabel* preampLabel = nullptr;
    QCheckBox* preampCheckBox = nullptr;

    // 第三组：RBW 配置
    QLabel* rbwModeLabel = nullptr;
    QComboBox* rbwModeCombo = nullptr;

    QLabel* rbwValueLabel = nullptr;
    QDoubleSpinBox* rbwSpinBox = nullptr;
    QLabel* rbwUnitLabel = nullptr;

    // 第四组：VBW 配置
    QLabel* vbwModeLabel = nullptr;
    QComboBox* vbwModeCombo = nullptr;

    QLabel* vbwValueLabel = nullptr;
    QDoubleSpinBox* vbwSpinBox = nullptr;
    QLabel* vbwUnitLabel = nullptr;

    // 第五组：耦合方式
    QLabel* couplingLabel = nullptr;
    QComboBox* couplingCombo = nullptr;

    // 第六组：测量模式
    QLabel* measurementModeLabel = nullptr;
    QComboBox* measurementModeCombo = nullptr;

    // 按钮
    QPushButton* okButton = nullptr;
    QPushButton* cancelButton = nullptr;

    //// 第七组：幅度和分辨率
    //QLabel* amplitudeLabel = nullptr;
    //QDoubleSpinBox* amplitudeSpinBox = nullptr;
    //QLabel* amplitudeUnitLabel = nullptr;

    //QLabel* resolutionLabel = nullptr;
    //QDoubleSpinBox* resolutionSpinBox = nullptr;
    //QLabel* resolutionUnitLabel = nullptr;

    // ============ 内部数据 ============
    SpectrumAnalyzerConfig m_config;
};

#endif // SPECTRUMANALYZERCONFIGDIALOG_H