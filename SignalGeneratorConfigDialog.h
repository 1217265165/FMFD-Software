#ifndef SIGNALGENERATORCONFIGDIALOG_H
#define SIGNALGENERATORCONFIGDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include "CommonTypes.h"  // 包含结构体定义

// 不要定义 struct SignalGeneratorConfig，因为已在 CommonTypes.h 中定义

// 信号发生器配置对话框类
class SignalGeneratorConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignalGeneratorConfigDialog(QWidget* parent = nullptr);
    ~SignalGeneratorConfigDialog() = default;

    // 设置配置数据到对话框
    void setConfig(const SignalGeneratorConfig& config);

    // 从对话框获取配置数据
    SignalGeneratorConfig getConfig() const;

private:
    // 初始化 UI
    void setupUI();

    // 从对话框更新配置数据
    void updateConfigFromUI();

    // 加载配置数据到对话框
    void loadConfigToUI();

    // 槽函数 - 调制模式变化
    private slots:
        void onModulationModeChanged(int index);

    // 槽函数 - 确定按钮
    private slots:
        void onOkClicked();

    // 槽函数 - 取消按钮
    private slots:
        void onCancelClicked();

    // 槽函数 - 输出启用变化
    private slots:
        void onOutputEnabledChanged(int state);

private:
    // ============ UI 控件 ============

    // 第一组：功率配置
    QLabel* powerLabel = nullptr;
    QDoubleSpinBox* powerSpinBox = nullptr;
    QLabel* powerUnitLabel = nullptr;

    // 第二组：补偿设置
    QLabel* frequencyCompensationLabel = nullptr;
    QCheckBox* frequencyCompensationCheckBox = nullptr;

    QLabel* powerCompensationLabel = nullptr;
    QCheckBox* powerCompensationCheckBox = nullptr;

    // 第三组：输出连接器
    QLabel* outputConnectorLabel = nullptr;
    QComboBox* outputConnectorCombo = nullptr;

    // 第四组：功率平坦化
    QLabel* powerFlatteningLabel = nullptr;
    QCheckBox* powerFlatteningCheckBox = nullptr;

    // 第五组：调制模式
    QLabel* modulationModeLabel = nullptr;
    QComboBox* modulationModeCombo = nullptr;

    // 第六组：输出控制
    QLabel* outputControlLabel = nullptr;
    QCheckBox* outputEnabledCheckBox = nullptr;

    // 按钮
    QPushButton* okButton = nullptr;
    QPushButton* cancelButton = nullptr;

    // ============ 内部数据 ============
    SignalGeneratorConfig m_config;
};

#endif // SIGNALGENERATORCONFIGDIALOG_H
