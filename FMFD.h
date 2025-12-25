#pragma once
#pragma execution_character_set("utf-8")
#include <QMainWindow>
#include <QWidget>

#include <QCoreApplication>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QProcess>
#include <QMap>
#include <memory>
#include "SchemeManager.h"
#include "CommonTypes.h"
#include "DataAcquisitionService.h"
#include "SpectrumAnalyzerConfigDialog.h"
#include "SignalGeneratorConfigDialog.h"
#include "SchemeManagementDialog.h"
#include "UsageDialog.h"

class ZoomableGraphicsView;
class AutoTestModule;
class BRBEngine;
class QButtonGroup;
class QRadioButton;
class QAbstractButton;
class DataAcquisitionService;
class SpectrumAnalyzerConfigDialog;
class SignalGeneratorConfigDialog;
class QVBoxLayout;  // Forward declaration for m_diagScrollLayout
struct MeasurementData;

// ============ 频率扫描配置对话框 ============

class FrequencySweepConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FrequencySweepConfigDialog(QWidget* parent = nullptr);
    FrequencySweepConfig getConfig() const { return m_config; }
    void setConfig(const FrequencySweepConfig& config);


private slots:
    void onSweepModeChanged(int index);
    void onAddSegment();
    void onRemoveSegment();
    void onBrowseFreqFile();
    void updateConfigFromUI();

private:
    void setupUI();
    void loadSegmentsToTable();

    FrequencySweepConfig m_config;

    // UI控件
    QComboBox* m_sweepModeCombo = nullptr;
    QTableWidget* m_segmentTable = nullptr;
    QPushButton* m_addSegmentBtn = nullptr;
    QPushButton* m_removeSegmentBtn = nullptr;
    QLineEdit* m_freqFileEdit = nullptr;
    QPushButton* m_browseFileBtn = nullptr;
    //QLineEdit* m_configPowerEdit = nullptr;
    //QLineEdit* m_configRbwEdit = nullptr;
    //QLineEdit* m_configVbwEdit = nullptr;
    QPushButton* m_okBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
};

// ============ 主窗口 ============

class FMFD : public QMainWindow
{
    Q_OBJECT

public:
    explicit FMFD(QWidget* parent = nullptr);
    ~FMFD() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    // ============ 仪器资源和参数控件 ============
    QLineEdit* m_sgResourceEdit = nullptr;
    QLineEdit* m_saResourceEdit = nullptr;
    QLineEdit* m_freqListEdit = nullptr;
    QLineEdit* m_powerListEdit = nullptr;
    QLineEdit* m_attenListEdit = nullptr;
    QLineEdit* m_repeatsEdit = nullptr;
    QLineEdit* m_rbwEdit = nullptr;
    QLineEdit* m_spanEdit = nullptr;
    QLineEdit* m_vbwEdit = nullptr;

    // ============ 按钮控件 ============
    QPushButton* m_startBtn = nullptr;
    QPushButton* m_oneClickBtn = nullptr;
    QPushButton* m_stopBtn = nullptr;

    // ============ 显示控件 ============
    QTableWidget* m_featureTable = nullptr;
    QTextEdit* m_diagText = nullptr;
    ZoomableGraphicsView* m_structureView = nullptr;
    QMap<QString, QProgressBar*> m_moduleBars;
    QMap<QString, QGraphicsRectItem*> m_graphicsItems;
    QWidget* m_diagScrollContent = nullptr;  // 用于动态重建进度条列表
    QVBoxLayout* m_diagScrollLayout = nullptr;  // 用于动态重建进度条列表

    // ============ 业务逻辑类 ============
    AutoTestModule* m_autoTest = nullptr;
    BRBEngine* m_brbEngine = nullptr;
    QStringList m_excludedModules;

    // 数据采集服务
    std::unique_ptr<DataAcquisitionService> m_dataAcquisitionService;

    // ============ Python集成 ============
    QProcess* m_pyProc = nullptr;
    QString m_pythonExe = QStringLiteral("python");
    //QString m_vizScript = QStringLiteral("D:/PycharmProjects/FMFD/viz-cli.py");

    QString m_vizScript = QCoreApplication::applicationDirPath() + "/viz/viz-cli.exe";
    // 用于并发控制：当一个 viz 进程在运行时，保存最新的挂起 mode（-1 表示无挂起）
    int m_pendingVizMode = -1;

    // ============ BRB诊断集成 ============
    QProcess* m_brbProc = nullptr;
    QString m_brbExePath;     // BRB诊断exe路径，通过配置对话框设置或使用默认路径

    // ============ 图像显示 ============
    QLabel* m_instrImage = nullptr;
    QButtonGroup* m_imageButtonGroup = nullptr;


    // 方案
    TestScheme m_currentScheme;
    SchemeManagementDialog* m_schemeDialog = nullptr;
    std::unique_ptr<SchemeManager> m_schemeManager;  // ✅ 这个位置是正确的

    // ============ 配置对话框 ============
    FrequencySweepConfigDialog* m_configDialog = nullptr;
    SpectrumAnalyzerConfigDialog* m_saConfigDialog = nullptr;
    SignalGeneratorConfigDialog* m_sgConfigDialog = nullptr;

    // ============ 当前配置 ============
    FrequencySweepConfig m_currentConfig;
    SpectrumAnalyzerConfig m_currentSaConfig;
    SignalGeneratorConfig m_currentSgConfig;



private slots:
    // ============ 一键采集相关 ============
    void onOneClickStartClicked();

    // ============ 自定义参数采集相关 ============
    void onStartTestClicked();

    // ============ 采集控制相关 ============
    void stopAutomatedTest();

    // ============ 数据采集服务回调 ============
    void onStatusMessage(const QString& message);
    void onMeasurementData(const MeasurementData& data);
    void onFrequencyResponse(double freqHz, double amplitude);
    void onAutomatedTestFinished();
    void onNewMeasurement(const QMap<QString, double>& features);

    // ============ Python可视化集成 ============
    void requestPythonVisualization(int mode);
    void onPythonFinished(int exitCode, QProcess::ExitStatus status);
    void onPythonReadyRead();

    // ============ BRB诊断集成 ============
    void runBRBDiagnosis();
    void onBRBDiagnosisFinished(int exitCode, QProcess::ExitStatus status);
    void onBRBDiagnosisReadyRead();
    void configureBRBPaths();

    // ============ 图像切换相关 ============
    void onImageButtonClicked(QAbstractButton* button);



    // ============ 配置对话框相关 ============
    void openFrequencySweepConfig();
    void onConfigDialogAccepted();
    // ============ 频谱分析仪配置 ============
    void openSpectrumAnalyzerConfig();
    void onSaConfigAccepted();

    // ============ 信号发生器配置 ============
    void openSignalGeneratorConfig();
    void onSgConfigAccepted();

    // ============ 方案管理相关 ============

    void openSchemeManagement();
    void onSchemeLoaded();
    void saveSchemeDialog();
    void loadSchemeDialog();
    void deleteSchemeDialog();
    void onSchemeSaved();

    // ✅ 新增：处理方案管理对话框的配置编辑请求
    void onSchemeEditFrequencySweep();
    void onSchemeEditSpectrumAnalyzer();
    void onSchemeEditSignalGenerator();

    void openUsageDialog();

private:
    // ============ UI初始化 ============
    void setupUi();
    void setupConnections();

    // ============ 辅助函数 ============
    void refreshFeatureTable(const QMap<QString, double>& features);

    // ============ 自动化测试方法 ============
    void startAutomatedTest(const QStringList& frequencies, double powerDbm, double rbw,
        double span, const QString& vbwMode, int repeats);


};