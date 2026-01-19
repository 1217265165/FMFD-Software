#pragma execution_character_set("utf-8")

#include "FMFD.h"
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QGroupBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QRegularExpression>
#include <QProcess>
#include <QStandardPaths>
#include <QFileInfo>
#include <QPixmap>
#include <QDir>
#include <QCoreApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QScrollArea>
#include <QButtonGroup>
#include <QRadioButton>
#include <QComboBox>
#include <QFileDialog>
#include <QDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QSettings>
#include <QPainter>
#include <QTextStream>
#include <QDateTime>
#include "CommonTypes.h"
#include "brbengine.h"
#include "ZoomableGraphicsView.h"
#include "DataAcquisitionService.h"
#include "SpectrumAnalyzerConfigDialog.h"
#include <qinputdialog.h>
#include <QTimer>



// ======================== FrequencySweepConfigDialog 实现 ========================

FrequencySweepConfigDialog::FrequencySweepConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("频率扫描配置 (Frequency Sweep Configuration)"));
    setMinimumWidth(600);
    setMinimumHeight(400);
    setupUI();
}

void FrequencySweepConfigDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 模式选择
    QHBoxLayout* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel(tr("采集模式 (Mode):")));
    m_sweepModeCombo = new QComboBox(this);
    m_sweepModeCombo->addItem(tr("分段生成 (Segmented)"), 0);
    m_sweepModeCombo->addItem(tr("文件导入 (File Import)"), 1);
    m_sweepModeCombo->setCurrentIndex(0);
    modeLayout->addWidget(m_sweepModeCombo);
    modeLayout->addStretch();
    mainLayout->addLayout(modeLayout);

    // 分段配置表格
    QLabel* segmentLabel = new QLabel(tr("分段参数 (Start_Hz, Stop_Hz, Step_Hz):"));
    mainLayout->addWidget(segmentLabel);
    m_segmentTable = new QTableWidget(this);
    m_segmentTable->setColumnCount(3);
    m_segmentTable->setHorizontalHeaderLabels(QStringList() << tr("Start (Hz)") << tr("Stop (Hz)") << tr("Step (Hz)"));
    m_segmentTable->horizontalHeader()->setStretchLastSection(true);
    m_segmentTable->setMaximumHeight(200);
    m_segmentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(m_segmentTable);

    QHBoxLayout* segmentBtnLayout = new QHBoxLayout();
    m_addSegmentBtn = new QPushButton(tr("+ 添加分段"), this);
    m_removeSegmentBtn = new QPushButton(tr("- 移除分段"), this);
    segmentBtnLayout->addWidget(m_addSegmentBtn);
    segmentBtnLayout->addWidget(m_removeSegmentBtn);
    segmentBtnLayout->addStretch();
    mainLayout->addLayout(segmentBtnLayout);

    // 文件导入配置
    QLabel* fileLabel = new QLabel(tr("频点文件 (Frequency File):"));
    mainLayout->addWidget(fileLabel);
    QHBoxLayout* fileLayout = new QHBoxLayout();
    m_freqFileEdit = new QLineEdit(this);
    m_freqFileEdit->setPlaceholderText(tr("输入文件路径或点击浏览"));
    m_browseFileBtn = new QPushButton(tr("浏览..."), this);
    m_browseFileBtn->setMaximumWidth(80);
    fileLayout->addWidget(m_freqFileEdit);
    fileLayout->addWidget(m_browseFileBtn);
    mainLayout->addLayout(fileLayout);

    //// 通用采集参数
    //QLabel* commonParamLabel = new QLabel(tr("采集参数 (Common Parameters):"));
    //mainLayout->addWidget(commonParamLabel);
    //QGridLayout* commonLayout = new QGridLayout();

    //commonLayout->addWidget(new QLabel(tr("功率 (Power dBm):")), 0, 0);
    //m_configPowerEdit = new QLineEdit(this);
    //m_configPowerEdit->setText("0.0");
    //m_configPowerEdit->setMaximumWidth(100);
    //commonLayout->addWidget(m_configPowerEdit, 0, 1);

    //commonLayout->addWidget(new QLabel(tr("RBW (Hz):")), 0, 2);
    //m_configRbwEdit = new QLineEdit(this);
    //m_configRbwEdit->setText("10000.0");
    //m_configRbwEdit->setMaximumWidth(100);
    //commonLayout->addWidget(m_configRbwEdit, 0, 3);

    //commonLayout->addWidget(new QLabel(tr("VBW模式:")), 1, 0);
    //m_configVbwEdit = new QLineEdit(this);
    //m_configVbwEdit->setText("same");
    //m_configVbwEdit->setMaximumWidth(100);
    //commonLayout->addWidget(m_configVbwEdit, 1, 1);

    //commonLayout->setColumnStretch(1, 0);
    //commonLayout->setColumnStretch(3, 0);
    //mainLayout->addLayout(commonLayout);

    mainLayout->addStretch();

    // 按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_okBtn = new QPushButton(tr("确定 (OK)"), this);
    m_cancelBtn = new QPushButton(tr("取消 (Cancel)"), this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_okBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);

    // 连接信号槽
    connect(m_sweepModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FrequencySweepConfigDialog::onSweepModeChanged);
    connect(m_addSegmentBtn, &QPushButton::clicked, this, &FrequencySweepConfigDialog::onAddSegment);
    connect(m_removeSegmentBtn, &QPushButton::clicked, this, &FrequencySweepConfigDialog::onRemoveSegment);
    connect(m_browseFileBtn, &QPushButton::clicked, this, &FrequencySweepConfigDialog::onBrowseFreqFile);
    connect(m_okBtn, &QPushButton::clicked, this, [this]() {
        updateConfigFromUI();
        accept();
        });
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    // 初始化表格
    loadSegmentsToTable();

    // 初始化模式
    onSweepModeChanged(0);
}




void FrequencySweepConfigDialog::setConfig(const FrequencySweepConfig& config)
{
    m_config = config;

    // 恢复UI状态
    if (m_config.mode == FrequencySweepConfig::Mode::SegmentGeneration) {
        m_sweepModeCombo->setCurrentIndex(0);
    }
    else {
        m_sweepModeCombo->setCurrentIndex(1);
        m_freqFileEdit->setText(QString::fromStdString(m_config.freqFilePath));
    }

    //m_configPowerEdit->setText(QString::number(m_config.powerDbm));
    //m_configRbwEdit->setText(QString::number(m_config.rbw));
    //m_configVbwEdit->setText(QString::fromStdString(m_config.vbwMode));

    loadSegmentsToTable();
}

void FrequencySweepConfigDialog::loadSegmentsToTable()
{
    m_segmentTable->setRowCount(0);
    for (const auto& seg : m_config.segments) {
        int row = m_segmentTable->rowCount();
        m_segmentTable->insertRow(row);
        m_segmentTable->setItem(row, 0, new QTableWidgetItem(QString::number(seg.startHz, 'e', 2)));
        m_segmentTable->setItem(row, 1, new QTableWidgetItem(QString::number(seg.stopHz, 'e', 2)));
        m_segmentTable->setItem(row, 2, new QTableWidgetItem(QString::number(seg.stepHz, 'e', 2)));
    }
}

void FrequencySweepConfigDialog::onSweepModeChanged(int index)
{
    if (index == 0) {
        // 分段模式
        m_segmentTable->setEnabled(true);
        m_addSegmentBtn->setEnabled(true);
        m_removeSegmentBtn->setEnabled(true);
        m_freqFileEdit->setEnabled(false);
        m_browseFileBtn->setEnabled(false);
    }
    else {
        // 文件导入模式
        m_segmentTable->setEnabled(false);
        m_addSegmentBtn->setEnabled(false);
        m_removeSegmentBtn->setEnabled(false);
        m_freqFileEdit->setEnabled(true);
        m_browseFileBtn->setEnabled(true);
    }
}

void FrequencySweepConfigDialog::onAddSegment()
{
    int row = m_segmentTable->rowCount();
    m_segmentTable->insertRow(row);
    m_segmentTable->setItem(row, 0, new QTableWidgetItem("1.000000e+06"));
    m_segmentTable->setItem(row, 1, new QTableWidgetItem("1.000000e+07"));
    m_segmentTable->setItem(row, 2, new QTableWidgetItem("1.000000e+05"));
}

void FrequencySweepConfigDialog::onRemoveSegment()
{
    int row = m_segmentTable->currentRow();
    if (row >= 0) {
        m_segmentTable->removeRow(row);
    }
    else {
        QMessageBox::warning(this, tr("提示"), tr("请先选择要删除的行"));
    }
}

void FrequencySweepConfigDialog::onBrowseFreqFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("选择频点文件"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("文本文件 (*.txt);;CSV文件 (*.csv);;所有文件 (*)"));
    if (!filePath.isEmpty()) {
        m_freqFileEdit->setText(filePath);
    }
}

void FrequencySweepConfigDialog::updateConfigFromUI()
{
    // 更新功率、RBW、VBW
    bool ok;
    //m_config.powerDbm = m_configPowerEdit->text().toDouble(&ok);
    //if (!ok) m_config.powerDbm = 0.0;

    //m_config.rbw = m_configRbwEdit->text().toDouble(&ok);
    //if (!ok) m_config.rbw = 10000.0;

    //m_config.vbwMode = m_configVbwEdit->text().trimmed().toStdString();
    //if (m_config.vbwMode.empty()) m_config.vbwMode = "same";

    // 更新采集模式和相关参数
    int modeIndex = m_sweepModeCombo->currentIndex();
    if (modeIndex == 0) {
        // 分段模式
        m_config.mode = FrequencySweepConfig::Mode::SegmentGeneration;
        m_config.segments.clear();

        for (int i = 0; i < m_segmentTable->rowCount(); ++i) {
            QTableWidgetItem* startItem = m_segmentTable->item(i, 0);
            QTableWidgetItem* stopItem = m_segmentTable->item(i, 1);
            QTableWidgetItem* stepItem = m_segmentTable->item(i, 2);

            if (!startItem || !stopItem || !stepItem) continue;

            bool ok1, ok2, ok3;
            double start = startItem->text().toDouble(&ok1);
            double stop = stopItem->text().toDouble(&ok2);
            double step = stepItem->text().toDouble(&ok3);

            if (ok1 && ok2 && ok3 && step > 0) {
                m_config.segments.push_back({ start, stop, step });
            }
        }
    }
    else {
        // 文件导入模式
        m_config.mode = FrequencySweepConfig::FileImport;
        m_config.freqFilePath = m_freqFileEdit->text().trimmed().toStdString();
    }
}

// ======================== FMFD 主窗口实现 ========================

FMFD::FMFD(QWidget* parent)
    : QMainWindow(parent)
{
    m_brbEngine = new BRBEngine(this);
    m_pyProc = new QProcess(this);
    m_brbProc = new QProcess(this);  // 初始化BRB诊断进程
    m_dataAcquisitionService = std::make_unique<DataAcquisitionService>();

    // 清除旧的错误设置（可选，仅第一次需要）
    QSettings settings("FMFD", "FMFD-Software");
     //settings.remove("BRB/ExePath");  // 取消注释可清除一次

    // 现在使用正确的路径拼接方式
    QDir appDir(QCoreApplication::applicationDirPath());
    m_brbPythonPath = settings.value("BRB/PythonPath", "python").toString();
    m_brbScriptPath = settings.value("BRB/ScriptPath", "D:/PycharmProjects/FMFD/FMFD/brb_diagnosis_cli.py").toString();
    m_brbExePath = settings.value("BRB/ExePath", appDir.filePath("BRB/brb_diagnosis.exe")).toString();

    // 初始化配置
    m_currentConfig.mode = FrequencySweepConfig::Mode::SegmentGeneration;
    m_currentConfig.segments = {
        {10e3, 1e6, 10e3},
        {1e6, 10e6, 1e6},
        {10e6, 500e6, 1e7},
        {500e6, 6.7e10, 50e6}
    };
    //m_currentConfig.powerDbm = 0.0;
    //m_currentConfig.rbw = 10000.0;
    //m_currentConfig.vbwMode = "same";

    setupUi();

    // ============ 添加以下代码 ============
    // 初始化方案管理器（方案CSV文件保存在 resource_files/config 目录）
    m_schemeManager = std::make_unique<SchemeManager>("resource_files/config");

    // 初始化当前方案
    m_currentScheme.name = "Default";
    m_currentScheme.sweepConfig = m_currentConfig;
    m_currentScheme.saConfig = m_currentSaConfig;
    m_currentScheme.sgConfig = m_currentSgConfig;

    // 初始化方案管理对话框
    m_schemeDialog = new SchemeManagementDialog(this);
    m_schemeDialog->setCurrentScheme(m_currentScheme);

    // ✅ 连接方案管理对话框的配置编辑信号
    connect(m_schemeDialog, &SchemeManagementDialog::requestEditFrequencySweep,
        this, &FMFD::onSchemeEditFrequencySweep);
    connect(m_schemeDialog, &SchemeManagementDialog::requestEditSpectrumAnalyzer,
        this, &FMFD::onSchemeEditSpectrumAnalyzer);
    connect(m_schemeDialog, &SchemeManagementDialog::requestEditSignalGenerator,
        this, &FMFD::onSchemeEditSignalGenerator);

    setupConnections();
    requestPythonVisualization(0);
}

FMFD::~FMFD()
{
    if (m_dataAcquisitionService) {
        m_dataAcquisitionService->stopMeasurement();
    }
}

void FMFD::setupUi()
{
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu* fileMenu = menuBar->addMenu(tr("文件"));
    QAction* exitAction = new QAction(tr("退出"), this);
    fileMenu->addAction(exitAction);
    connect(exitAction, &QAction::triggered, this, &FMFD::close);

    QMenu* actionMenu = menuBar->addMenu(tr("操作"));
    QAction* startAction = new QAction(tr("开始测试"), this);
    QAction* stopAction = new QAction(tr("停止测试"), this);
    actionMenu->addAction(startAction);
    actionMenu->addAction(stopAction);
    actionMenu->addSeparator();

    // ============ BRB诊断 ============
    QAction* brbDiagnosisAction = new QAction(tr("运行BRB诊断 (Python)"), this);
    actionMenu->addAction(brbDiagnosisAction);
    connect(brbDiagnosisAction, &QAction::triggered, this, &FMFD::runBRBDiagnosis);

    // 方案：在 setupUi() 的菜单设置中添加
    QMenu* schemeMenu = menuBar->addMenu(tr("方案"));
    QAction* openSchemeAction = new QAction(tr("方案管理"), this);
    schemeMenu->addAction(openSchemeAction);
    connect(openSchemeAction, &QAction::triggered, this, &FMFD::openSchemeManagement);


    // ============ 配置菜单 ============
    QMenu* configMenu = menuBar->addMenu(tr("配置"));
    QAction* frequencySweepConfigAction = new QAction(tr("频率扫描配置"), this);
    configMenu->addAction(frequencySweepConfigAction);
    connect(frequencySweepConfigAction, &QAction::triggered, this, &FMFD::openFrequencySweepConfig);

    // ============ 频谱分析仪参数配置（第一步新增）============
    QAction* saConfigAction = new QAction(tr("频谱分析仪参数配置"), this);
    configMenu->addAction(saConfigAction);
    connect(saConfigAction, &QAction::triggered, this, &FMFD::openSpectrumAnalyzerConfig);

    // ============ 信号发生器参数配置（预留）============
    QAction* sgConfigAction = new QAction(tr("信号发生器参数配置"), this);
    configMenu->addAction(sgConfigAction);
    connect(sgConfigAction, &QAction::triggered, this, &FMFD::openSignalGeneratorConfig);

    configMenu->addSeparator();

    // ============ BRB Python路径配置 ============
    QAction* brbPathConfigAction = new QAction(tr("BRB Python路径配置"), this);
    configMenu->addAction(brbPathConfigAction);
    connect(brbPathConfigAction, &QAction::triggered, this, &FMFD::configureBRBPaths);

    QMenu* viewMenu = menuBar->addMenu(tr("查看结构图"));
    QAction* mainAct = new QAction(tr("主结构图"), this);
    QAction* s1 = new QAction(tr("频率精度异常"), this);
    QAction* s2 = new QAction(tr("幅度测量不准确"), this);
    QAction* s3 = new QAction(tr("相位噪声增加"), this);
    QAction* s4 = new QAction(tr("参考电平失准"), this);
    viewMenu->addAction(mainAct);
    viewMenu->addAction(s1);
    viewMenu->addAction(s2);
    viewMenu->addAction(s3);
    viewMenu->addAction(s4);
    connect(mainAct, &QAction::triggered, [this]() { requestPythonVisualization(0); });
    connect(s1, &QAction::triggered, [this]() { requestPythonVisualization(1); });
    connect(s2, &QAction::triggered, [this]() { requestPythonVisualization(2); });
    connect(s3, &QAction::triggered, [this]() { requestPythonVisualization(3); });
    connect(s4, &QAction::triggered, [this]() { requestPythonVisualization(4); });

    QMenu* helpMenu = menuBar->addMenu(tr("帮助"));

    // 使用说明
    QAction* usageAction = new QAction(tr("使用说明"), this);  // ← 加 *
    helpMenu->addAction(usageAction);
    connect(usageAction, &QAction::triggered, this, &FMFD::openUsageDialog);

    // 分隔符
    helpMenu->addSeparator();

    // 关于
    QAction* aboutAction = new QAction(tr("关于"), this);  // ← 加 *
    helpMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("关于"), tr("FMFD 1.0 - UPC FMFD"));
        });


    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    // 控件布局区域
    QGroupBox* resBox = new QGroupBox(tr("Instruments (VISA resources)"), this);
    QHBoxLayout* resLayout = new QHBoxLayout();
    m_sgResourceEdit = new QLineEdit(this);
    //TCPIP::192.168.1.200::5025::SOCKET 
    m_sgResourceEdit->setText(tr("TCPIP0::192.168.1.200::INSTR"));
    m_saResourceEdit = new QLineEdit(this);
    //TCPIP::192.168.1.100::5025::SOCKET
    m_saResourceEdit->setText(tr("TCPIP0::192.168.1.100::INSTR"));
    resLayout->addWidget(new QLabel(tr("SG:")));
    resLayout->addWidget(m_sgResourceEdit);
    resLayout->addWidget(new QLabel(tr("SA:")));
    resLayout->addWidget(m_saResourceEdit);
    resBox->setLayout(resLayout);

    QGroupBox* paramsBox = new QGroupBox(tr("Scan / Single Test Parameters"), this);
    QHBoxLayout* paramsLayout = new QHBoxLayout();
    m_freqListEdit = new QLineEdit(this);
    m_freqListEdit->setPlaceholderText(tr("freqs Hz comma-separated, e.g. 1e9,1.1e9"));
    m_powerListEdit = new QLineEdit(this);
    m_powerListEdit->setPlaceholderText(tr("powers dBm comma-separated, e.g. -10,0"));
    m_attenListEdit = new QLineEdit(this);
    m_attenListEdit->setPlaceholderText(tr("atten dB list, e.g. 0,10"));
    m_repeatsEdit = new QLineEdit(this);
    m_repeatsEdit->setPlaceholderText(tr("repeats (e.g. 1)"));
    m_rbwEdit = new QLineEdit(this);
    m_rbwEdit->setPlaceholderText(tr("RBW Hz (e.g. 1000)"));
    m_spanEdit = new QLineEdit(this);
    m_spanEdit->setPlaceholderText(tr("SPAN Hz (e.g. 1e6)"));
    m_vbwEdit = new QLineEdit(this);
    m_vbwEdit->setPlaceholderText(tr("VBW: same|smooth|100"));
    paramsLayout->addWidget(m_freqListEdit);
    paramsLayout->addWidget(m_powerListEdit);
    paramsLayout->addWidget(m_attenListEdit);
    paramsLayout->addWidget(m_repeatsEdit);
    paramsLayout->addWidget(m_rbwEdit);
    paramsLayout->addWidget(m_spanEdit);
    paramsLayout->addWidget(m_vbwEdit);
    paramsBox->setLayout(paramsLayout);

    m_startBtn = new QPushButton(tr("Start Internal Test"), this);
    m_oneClickBtn = new QPushButton(tr("One-Click Auto Test (C++)"), this);
    m_stopBtn = new QPushButton(tr("Stop Test"), this);
    m_stopBtn->setEnabled(false);

    m_featureTable = new QTableWidget(this);
    m_featureTable->setColumnCount(2);
    m_featureTable->setHorizontalHeaderLabels(QStringList() << tr("Feature") << tr("Value"));
    m_featureTable->horizontalHeader()->setStretchLastSection(true);
    m_featureTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_featureTable->verticalHeader()->setVisible(false);

    QGroupBox* diagBox = new QGroupBox(tr("Fault Diagnosis (BRB Output)"), this);
    QVBoxLayout* diagLayout = new QVBoxLayout(diagBox);
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    m_diagScrollContent = new QWidget();
    m_diagScrollLayout = new QVBoxLayout(m_diagScrollContent);
    QStringList modules = m_brbEngine->moduleNames();
    for (const QString& m : modules) {
        if (m_excludedModules.contains(m)) continue;
        QLabel* lbl = new QLabel(m, this);
        QProgressBar* bar = new QProgressBar(this);
        bar->setRange(0, 100);
        bar->setValue(0);
        m_moduleBars[m] = bar;
        QHBoxLayout* row = new QHBoxLayout();
        row->addWidget(lbl);
        row->addWidget(bar);
        m_diagScrollLayout->addLayout(row);
    }
    m_diagScrollLayout->addStretch();
    scrollArea->setWidget(m_diagScrollContent);
    diagLayout->addWidget(scrollArea);

    m_diagText = new QTextEdit(this);
    m_diagText->setReadOnly(true);

    m_structureView = new ZoomableGraphicsView(this);
    m_structureView->setMinimumHeight(100);
    m_structureView->setMinimumWidth(480);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(resBox);
    leftLayout->addWidget(paramsBox);
    leftLayout->addSpacing(6);
    leftLayout->addWidget(new QLabel(tr("Controls:"), this));
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addWidget(m_startBtn);
    btnRow->addWidget(m_oneClickBtn);
    btnRow->addWidget(m_stopBtn);
    leftLayout->addLayout(btnRow);
    leftLayout->addSpacing(10);

    QHBoxLayout* imageSwitchLayout = new QHBoxLayout();
    m_imageButtonGroup = new QButtonGroup(this);
    QRadioButton* imageBtn1 = new QRadioButton(tr("频响测试连接示意图"), this);
    imageBtn1->setChecked(true);
    m_imageButtonGroup->addButton(imageBtn1, 0);
    QRadioButton* imageBtn2 = new QRadioButton(tr("剩余响应测试连接示意图"), this);
    m_imageButtonGroup->addButton(imageBtn2, 1);
    QRadioButton* imageBtn3 = new QRadioButton(tr("频响曲线"), this);
    m_imageButtonGroup->addButton(imageBtn3, 2);
    imageSwitchLayout->addWidget(imageBtn1);
    imageSwitchLayout->addWidget(imageBtn2);
    imageSwitchLayout->addWidget(imageBtn3);
    imageSwitchLayout->addStretch();
    leftLayout->addLayout(imageSwitchLayout);

    m_instrImage = new QLabel(this);
    QPixmap iconPixmap("./resource_files/icons/连接示意图.png");

    m_instrImage->setScaledContents(true);
    m_instrImage->setPixmap(iconPixmap);
    m_instrImage->setAlignment(Qt::AlignCenter);
    int fixedHeight = int(this->height() * 0.2);
    m_instrImage->setMinimumHeight(fixedHeight);
    m_instrImage->setMaximumHeight(fixedHeight);
    leftLayout->addWidget(m_instrImage, 0, Qt::AlignHCenter);

    leftLayout->addWidget(new QLabel(tr("Measurement Features:"), this));
    leftLayout->addWidget(m_featureTable, 1);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(new QLabel(tr("System Structure (visual):"), this));
    rightLayout->addWidget(m_structureView, 2);
    rightLayout->addWidget(diagBox, 2);
    rightLayout->addWidget(new QLabel(tr("Log / Diagnosis:"), this));
    rightLayout->addWidget(m_diagText, 1);

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 1);

    central->setLayout(mainLayout);
    setWindowTitle(tr("FMFD - Integrated Test & BRB Diagnosis"));
    resize(1200, 760);

    int initialDiagHeight = this->height() * 0.3;
    diagBox->setMinimumHeight(initialDiagHeight);
}




void FMFD::setupConnections()
{
    connect(m_oneClickBtn, &QPushButton::clicked, this, &FMFD::onOneClickStartClicked);
    connect(m_startBtn, &QPushButton::clicked, this, &FMFD::onStartTestClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &FMFD::stopAutomatedTest);
    connect(m_imageButtonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &FMFD::onImageButtonClicked);
    connect(m_brbEngine, &BRBEngine::logMessage, [this](const QString& s) { m_diagText->append(s); });

    // 连接BRB诊断可视化信号 - 更新进度条和图形
    connect(m_brbEngine, &BRBEngine::diagnosisReady, this, [this](const QMap<QString, double>& moduleProb) {
        // 清空现有的进度条布局
        if (m_diagScrollLayout) {
            // 安全的删除方式：使用deleteLater递归删除所有子widget
            while (m_diagScrollLayout->count() > 0) {
                QLayoutItem* item = m_diagScrollLayout->takeAt(0);
                if (item) {
                    if (item->widget()) {
                        item->widget()->deleteLater();
                    }
                    else if (item->layout()) {
                        // 递归删除布局中的所有widget
                        while (item->layout()->count() > 0) {
                            QLayoutItem* subItem = item->layout()->takeAt(0);
                            if (subItem) {
                                if (subItem->widget()) {
                                    subItem->widget()->deleteLater();
                                }
                                // subItem will be cleaned up by Qt
                            }
                        }
                        // Layout item will be deleted with parent
                    }
                    // Don't delete item immediately when widgets use deleteLater
                }
            }

            // 清空m_moduleBars映射
            m_moduleBars.clear();

            // 按故障概率从高到低排序
            QList<QPair<QString, double>> sortedModules;
            for (auto it = moduleProb.constBegin(); it != moduleProb.constEnd(); ++it) {
                if (!m_excludedModules.contains(it.key())) {
                    sortedModules.append(qMakePair(it.key(), it.value()));
                }
            }
            std::sort(sortedModules.begin(), sortedModules.end(),
                [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
                    return a.second > b.second;  // 从高到低排序
                });

            // 重新创建进度条（按排序后的顺序）
            for (const auto& pair : sortedModules) {
                const QString& moduleName = pair.first;
                double prob = pair.second;

                QLabel* lbl = new QLabel(moduleName, m_diagScrollContent);
                QProgressBar* bar = new QProgressBar(m_diagScrollContent);
                bar->setRange(0, 100);
                int percentage = static_cast<int>(prob * 100);
                bar->setValue(percentage);

                // 根据概率设置颜色
                if (prob > 0.1) {
                    // 高概率用红色样式
                    bar->setStyleSheet("QProgressBar::chunk { background-color: #ff6666; }");
                }
                else {
                    // 低概率用默认样式
                    bar->setStyleSheet("");
                }

                m_moduleBars[moduleName] = bar;

                QHBoxLayout* row = new QHBoxLayout();
                row->addWidget(lbl);
                row->addWidget(bar);
                m_diagScrollLayout->addLayout(row);
            }

            m_diagScrollLayout->addStretch();
        }

        // 更新图形高亮显示（如果有图形项）
        for (auto it = m_graphicsItems.constBegin(); it != m_graphicsItems.constEnd(); ++it) {
            double prob = moduleProb.value(it.key(), 0.0);
            QColor color;
            if (prob > 0.1) {
                // 高概率用红色
                int intensity = static_cast<int>(prob * 255);
                color = QColor(255, 255 - intensity, 255 - intensity);
            }
            else {
                // 低概率用浅色
                color = QColor(240, 240, 240);
            }
            it.value()->setBrush(QBrush(color));
        }

        // 更新结构图可视化
        requestPythonVisualization(4);  // mode=4 触发symptom模式更新图形
        });

    connect(m_pyProc, &QProcess::readyReadStandardOutput, this, &FMFD::onPythonReadyRead);
    connect(m_pyProc, &QProcess::readyReadStandardError, this, &FMFD::onPythonReadyRead);
    connect(m_pyProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &FMFD::onPythonFinished);

    // BRB诊断进程连接
    connect(m_brbProc, &QProcess::readyReadStandardOutput, this, &FMFD::onBRBDiagnosisReadyRead);
    connect(m_brbProc, &QProcess::readyReadStandardError, this, &FMFD::onBRBDiagnosisReadyRead);
    connect(m_brbProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &FMFD::onBRBDiagnosisFinished);

    // 数据采集服务回调
    if (m_dataAcquisitionService) {
        m_dataAcquisitionService->setCallbacks(
            [this](const std::string& msg) {
                QMetaObject::invokeMethod(this, [this, msg]() {
                    onStatusMessage(QString::fromStdString(msg));
                    });
            },
            [this](const MeasurementData& data) {
                QMetaObject::invokeMethod(this, [this, data]() {
                    onMeasurementData(data);
                    }, Qt::QueuedConnection);
            },
            [this](double freqHz, double amplitude) {
                QMetaObject::invokeMethod(this, [this, freqHz, amplitude]() {
                    onFrequencyResponse(freqHz, amplitude);
                    }, Qt::QueuedConnection);
            }
        );
        
        // 设置完成回调
        m_dataAcquisitionService->setCompletionCallback(
            [this]() {
                QMetaObject::invokeMethod(this, [this]() {
                    onAutomatedTestFinished();
                    }, Qt::QueuedConnection);
            }
        );
    }

    // 配置对话框
    m_configDialog = new FrequencySweepConfigDialog(this);
    m_configDialog->setConfig(m_currentConfig);
    connect(m_configDialog, &QDialog::accepted, this, &FMFD::onConfigDialogAccepted);

    // ============ 频谱分析仪配置对话框初始化（第一步新增）============
    m_saConfigDialog = new SpectrumAnalyzerConfigDialog(this);
    connect(m_saConfigDialog, &QDialog::accepted, this, &FMFD::onSaConfigAccepted);

    // ============ 新增：信号发生器配置对话框初始化 ============
    m_sgConfigDialog = new SignalGeneratorConfigDialog(this);
    m_sgConfigDialog->setConfig(m_currentSgConfig);
    connect(m_sgConfigDialog, &QDialog::accepted, this, &FMFD::onSgConfigAccepted);
}

void FMFD::openFrequencySweepConfig()
{
    if (!m_configDialog) {
        m_configDialog = new FrequencySweepConfigDialog(this);
    }
    m_configDialog->setConfig(m_currentConfig);
    m_configDialog->exec();
}

void FMFD::onConfigDialogAccepted()
{
    if (m_configDialog) {
        m_currentConfig = m_configDialog->getConfig();
        m_currentScheme.sweepConfig = m_currentConfig;  // ✅ 同步到当前方案
        m_diagText->append("[配置] 频率扫描参数已更新");
    }
}

// ============ 频谱分析仪配置相关（第一步新增）============

void FMFD::openSpectrumAnalyzerConfig()
{
    if (m_saConfigDialog) {
        m_saConfigDialog->setConfig(m_currentSaConfig);
        m_saConfigDialog->exec();
    }
}

void FMFD::onSaConfigAccepted()
{
    if (m_saConfigDialog) {
        m_currentSaConfig = m_saConfigDialog->getConfig();
        m_currentScheme.saConfig = m_currentSaConfig;  // ✅ 同步到当前方案
        m_diagText->append("[配置] 频谱分析仪参数已更新");

        // 显示所有参数（包括新增的 amplitude 和 resolution）
        m_diagText->append(QString("  - 参考电平: %1 dBm").arg(m_currentSaConfig.refLevelDbm, 0, 'f', 1));
        m_diagText->append(QString("  - 衰减器: %1 dB").arg(m_currentSaConfig.attenuatorDb));
        m_diagText->append(QString("  - 前置放大: %1").arg(m_currentSaConfig.preampEnabled ? "启用" : "禁用"));
        m_diagText->append(QString("  - RBW: %1 Hz").arg(m_currentSaConfig.rbwHz, 0, 'e', 2));
        m_diagText->append(QString("  - VBW: %1 Hz").arg(m_currentSaConfig.vbwHz, 0, 'e', 2));
        m_diagText->append(QString("  - 耦合: %1").arg(m_currentSaConfig.couplingMode));
        m_diagText->append(QString("  - 模式: %1").arg(m_currentSaConfig.measurementMode));

        // ============ 新增：幅度和分辨率 ============
        m_diagText->append(QString("  - Amplitude: %1 dBm").arg(m_currentSaConfig.amplitudeDbm, 0, 'f', 1));
        m_diagText->append(QString("  - Resolution: %1 Hz").arg(m_currentSaConfig.resolutionHz, 0, 'e', 2));
    }
}


// ============ 信号发生器配置相关（预留第二步）============

void FMFD::openSignalGeneratorConfig()
{
    if (!m_sgConfigDialog) {
        m_sgConfigDialog = new SignalGeneratorConfigDialog(this);
    }
    m_sgConfigDialog->setConfig(m_currentSgConfig);
    m_sgConfigDialog->exec();
}

void FMFD::onSgConfigAccepted()
{
    if (m_sgConfigDialog) {
        m_currentSgConfig = m_sgConfigDialog->getConfig();
        m_currentScheme.sgConfig = m_currentSgConfig;  // ✅ 同步到当前方案
        m_diagText->append("[配置] 信号发生器参数已更新");
        m_diagText->append(QString("  - 输出功率: %1 dBm").arg(m_currentSgConfig.powerDbm, 0, 'f', 1));
        m_diagText->append(QString("  - 频率补偿: %1").arg(m_currentSgConfig.frequencyCompensationEnabled ? "启用" : "禁用"));
        m_diagText->append(QString("  - 功率补偿: %1").arg(m_currentSgConfig.powerCompensationEnabled ? "启用" : "禁用"));
        m_diagText->append(QString("  - 输出连接器: %1").arg(m_currentSgConfig.outputConnector));
        m_diagText->append(QString("  - 功率平坦化: %1").arg(m_currentSgConfig.powerFlatteningEnabled ? "启用" : "禁用"));
        m_diagText->append(QString("  - 调制模式: %1").arg(m_currentSgConfig.modulationMode));
        m_diagText->append(QString("  - 输出状态: %1").arg(m_currentSgConfig.outputEnabled ? "启用" : "禁用"));
    }
}


void FMFD::onOneClickStartClicked()
{
    QString sg = m_sgResourceEdit->text().trimmed();
    QString sa = m_saResourceEdit->text().trimmed();
    if (sg.isEmpty() || sa.isEmpty()) {
        QMessageBox::warning(this, tr("Missing resources"), tr("Please fill SG and SA resource strings."));
        return;
    }
    
    // 清除之前的频响数据
    clearFrequencyResponseData();
    
    m_diagText->append("[自动采集] 初始化仪器...");
    bool ok = m_dataAcquisitionService->initialize(sg.toStdString(), sa.toStdString());
    if (!ok) {
        m_diagText->append("[错误] 仪器初始化失败！");
        return;
    }
    m_diagText->append("[自动采集] 一键自动频响采集开始...");
    if (!m_dataAcquisitionService->startAutoFrequencyResponseTest()) {
        m_diagText->append("[错误] 自动采集启动失败！");
        return;
    }
    m_oneClickBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
}

void FMFD::onStartTestClicked()
{
    auto parseList = [](const QString& s)->QStringList {
        if (s.trimmed().isEmpty()) return QStringList();
        QRegularExpression re("[,;\\s]+");
        return s.split(re, Qt::SkipEmptyParts);
        };
    QStringList freqs = parseList(m_freqListEdit->text());
    QStringList powerStrs = parseList(m_powerListEdit->text());
    QList<double> powers;
    for (const QString& p : powerStrs) {
        bool ok = false;
        double v = p.toDouble(&ok);
        if (ok) powers.append(v);
    }
    int repeats = 1;
    bool okr = false;
    int r = m_repeatsEdit->text().toInt(&okr);
    if (okr) repeats = r;
    double rbw = 1000.0;
    bool okb = false;
    double rbv = m_rbwEdit->text().toDouble(&okb);
    if (okb) rbw = rbv;
    double span = 1e6;
    bool oks = false;
    double spv = m_spanEdit->text().toDouble(&oks);
    if (oks) span = spv;
    QString vbw = m_vbwEdit->text().trimmed();
    if (vbw.isEmpty()) vbw = "smooth";
    double powerDbm = -10.0;
    if (!powers.isEmpty()) {
        powerDbm = powers.first();
    }
    startAutomatedTest(freqs, powerDbm, rbw, span, vbw, repeats);
}

void FMFD::startAutomatedTest(const QStringList& frequencies, double powerDbm, double rbw, double span, const QString& vbwMode, int repeats)
{
    // 清除之前的频响数据
    clearFrequencyResponseData();
    
    if (!m_dataAcquisitionService) {
        m_diagText->append("Error: Data acquisition service not initialized");
        return;
    }
    QString sg = m_sgResourceEdit->text().trimmed();
    QString sa = m_saResourceEdit->text().trimmed();
    bool initSuccess = m_dataAcquisitionService->initialize(sg.toStdString(), sa.toStdString());
    if (!initSuccess) {
        m_diagText->append("Error: Instrument initialization failed");
        return;
    }
    std::vector<double> freqVector;
    for (const QString& freqStr : frequencies) {
        bool ok = false;
        double freq = freqStr.toDouble(&ok);
        if (ok) {
            freqVector.push_back(freq);
        }
    }
    if (freqVector.empty()) {
        m_diagText->append("Error: No valid frequency values");
        return;
    }
    m_diagText->append(QString("Starting automated test - Frequencies: %1, Power: %2 dBm, RBW: %3 Hz").arg(freqVector.size()).arg(powerDbm).arg(rbw));
    bool testStarted = m_dataAcquisitionService->startFrequencyResponseTest(freqVector, powerDbm, rbw, vbwMode.toStdString());
    if (testStarted) {
        m_diagText->append("Automated test started successfully");
        m_oneClickBtn->setEnabled(false);
        m_stopBtn->setEnabled(true);
    }
    else {
        m_diagText->append("Error: Failed to start test");
    }
}

void FMFD::stopAutomatedTest()
{
    if (m_dataAcquisitionService) {
        m_dataAcquisitionService->stopMeasurement();
        m_diagText->append("Test stopped by user");
    }
    m_oneClickBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
}

void FMFD::onStatusMessage(const QString& message)
{
    m_diagText->append(QString("[STATUS] %1").arg(message));
}

void FMFD::onMeasurementData(const MeasurementData& data)
{
    QMap<QString, double> features;
    features["peak_power"] = data.peakDbm;
    features["peak_frequency"] = data.peakFreqHz;
    features["phase_noise"] = data.phaseNoiseDbcPerHz;
    features["ref_level"] = data.refLevelDbm;

    refreshFeatureTable(features);
    m_brbEngine->infer(features);

    m_diagText->append(QString("[DATA] Freq: %1 MHz, Peak: %2 dBm, Phase Noise: %3 dBc/Hz")
        .arg(data.peakFreqHz / 1e6, 0, 'f', 2)
        .arg(data.peakDbm, 0, 'f', 2)
        .arg(data.phaseNoiseDbcPerHz, 0, 'f', 2));
}

void FMFD::onFrequencyResponse(double freqHz, double amplitude)
{
    // 存储频响数据点
    m_frequencyResponseData.append(qMakePair(freqHz, amplitude));
    m_diagText->append(QString("[FREQ_RESP] %1 MHz: %2 dBm").arg(freqHz / 1e6, 0, 'f', 2).arg(amplitude, 0, 'f', 2));
}

void FMFD::onAutomatedTestFinished()
{
    m_diagText->append("Automated test completed");
    m_oneClickBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    
    // 测试结束后，自动切换到频响曲线显示
    if (!m_frequencyResponseData.isEmpty()) {
        // 选中频响曲线选项
        QAbstractButton* freqRespButton = m_imageButtonGroup->button(2);
        if (freqRespButton) {
            freqRespButton->setChecked(true);
            updateFrequencyResponsePlot();
            m_diagText->append(tr("[频响曲线] 测试完成，已自动切换到频响曲线显示"));
        }
    }
}

void FMFD::onNewMeasurement(const QMap<QString, double>& features)
{
    refreshFeatureTable(features);
    m_brbEngine->infer(features);
}

void FMFD::refreshFeatureTable(const QMap<QString, double>& features)
{
    m_featureTable->setRowCount(features.size());
    int r = 0;
    for (auto it = features.constBegin(); it != features.constEnd(); ++it, ++r) {
        m_featureTable->setItem(r, 0, new QTableWidgetItem(it.key()));
        m_featureTable->setItem(r, 1, new QTableWidgetItem(QString::number(it.value(), 'f', 4)));
    }
}


void FMFD::requestPythonVisualization(int mode)
{
    if (!m_pyProc) m_pyProc = new QProcess(this);

    // 如果已有进程在跑，记住最新请求并返回（等待当前进程结束后触发）
    if (m_pyProc->state() != QProcess::NotRunning) {
        m_diagText->append("Python process not exited, queueing visualization request for mode " + QString::number(mode));
        m_pendingVizMode = mode;
        return;
    }

    QString scriptPath = QCoreApplication::applicationDirPath() + "/viz/viz-cli.exe";
    // 保留相对 outPath（便于打包），但设置 working directory 与主程序一致
    QString outPath = "./resource_files/icons/fmfd_viz.png";

    // args 只包含选项，不要把 scriptPath 放进 args
    QStringList args;
    if (mode == 0) {
        args << "--mode" << "whole" << "--out" << outPath;
    }
    else {
        args << "--mode" << "symptom" << "--index" << QString::number(mode) << "--out" << outPath;
    }

    // 将子进程的工作目录设为应用目录，这样相对路径以 FMFD.exe 所在目录为基准
    QString appDir = QCoreApplication::applicationDirPath();
    m_pyProc->setWorkingDirectory(appDir);

    if (!QFile::exists(scriptPath)) {
        m_diagText->append("viz executable not found: " + scriptPath);
        return;
    }

    m_diagText->append(QString("Running: %1 %2").arg(scriptPath, args.join(' ')));
    // 设置程序与参数
    m_pyProc->setProgram(scriptPath);
    m_pyProc->setArguments(args);
    // 启动并清除上一次可能遗留的输出缓存
    m_pyProc->start();

    // 清理 scene，显示加载中状态
    m_structureView->setScene(new QGraphicsScene(this));
    m_diagText->append(QString("Started viz process (pid=%1)").arg((qulonglong)m_pyProc->processId()));
}
// onPythonReadyRead 增强诊断输出
void FMFD::onPythonReadyRead()
{
    if (!m_pyProc) return;
    QByteArray out = m_pyProc->readAllStandardOutput();
    QByteArray err = m_pyProc->readAllStandardError();
    if (!out.isEmpty()) {
        m_diagText->append(QString::fromLocal8Bit(out));
    }
    if (!err.isEmpty()) {
        m_diagText->append(QString::fromLocal8Bit(err));
    }
}

// onPythonFinished：加载相对路径的图片（与 working directory 对齐），并触发挂起请求
void FMFD::onPythonFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(status);
    m_diagText->append(QString("viz process finished (exit=%1)").arg(exitCode));

    // 使用与子进程相同的相对路径（FMFD.exe 当前目录下）
    QString outPath = "./resource_files/icons/fmfd_viz.png";
    QFileInfo fi(QCoreApplication::applicationDirPath() + "/" + outPath);
    if (!fi.exists()) {
        m_diagText->append(tr("Visualization image not found: %1").arg(fi.absoluteFilePath()));
        // 检查子进程输出以获取更多信息（onPythonReadyRead 已打印）
    }
    else {
        QPixmap pix(fi.absoluteFilePath());
        if (pix.isNull()) {
            m_diagText->append(tr("Failed to load visualization image: %1").arg(fi.absoluteFilePath()));
        }
        else {
            QGraphicsScene* scene = new QGraphicsScene(this);
            scene->addPixmap(pix);
            m_structureView->setScene(scene);
            m_structureView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
            m_diagText->append("Visualization loaded: " + fi.absoluteFilePath());
        }
    }

    // 如果有挂起的请求（最新的一次），立即处理并清空挂起标记
    if (m_pendingVizMode != -1) {
        int nextMode = m_pendingVizMode;
        m_pendingVizMode = -1;
        QTimer::singleShot(50, this, [this, nextMode]() { requestPythonVisualization(nextMode); });
    }
}

void FMFD::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    int fixedHeight = int(this->height() * 0.4);
    int checkedId = m_imageButtonGroup->checkedId();
    
    if (checkedId == 2) {
        // 频响曲线选项 - 重新绘制曲线以适应新尺寸
        updateFrequencyResponsePlot();
    }
    else {
        QString currentImagePath;
        if (checkedId == 0) {
            currentImagePath = "./resource_files/icons/频响.png";
        }
        else {
            currentImagePath = "./resource_files/icons/剩余响应.png";
        }
        QPixmap iconPixmap(currentImagePath);
        double aspect = iconPixmap.width() * 1.0 / iconPixmap.height();
        int scaledWidth = int(fixedHeight * aspect);
        m_instrImage->setPixmap(iconPixmap.scaled(scaledWidth, fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_instrImage->setMinimumHeight(fixedHeight);
        m_instrImage->setMaximumHeight(fixedHeight);
        m_instrImage->setMinimumWidth(scaledWidth);
        m_instrImage->setMaximumWidth(scaledWidth);
    }
    
    QList<QGroupBox*> diagBoxes = findChildren<QGroupBox*>(tr("Fault Diagnosis (BRB Output)"));
    if (!diagBoxes.isEmpty()) {
        QGroupBox* diagBox = diagBoxes.first();
        int newHeight = this->height() * 0.4;
        diagBox->setMinimumHeight(newHeight);
    }
}

//连接方式切换
void FMFD::onImageButtonClicked(QAbstractButton* button)
{
    int buttonId = m_imageButtonGroup->id(button);
    QString imagePath;
    if (buttonId == 0) {
        imagePath = "./resource_files/icons/频响.png";
    }
    else if (buttonId == 1) {
        imagePath = "./resource_files/icons/剩余响应.png";
    }
    else if (buttonId == 2) {
        // 频响曲线选项 - 显示频响曲线图
        updateFrequencyResponsePlot();
        return;
    }
    if (!imagePath.isEmpty()) {
        QPixmap iconPixmap(imagePath);
        if (!iconPixmap.isNull()) {
            int fixedHeight = int(this->height() * 0.4);
            double aspect = iconPixmap.width() * 1.0 / iconPixmap.height();
            int scaledWidth = int(fixedHeight * aspect);
            m_instrImage->setPixmap(iconPixmap.scaled(scaledWidth, fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_instrImage->setMinimumHeight(fixedHeight);
            m_instrImage->setMaximumHeight(fixedHeight);
            m_instrImage->setMinimumWidth(scaledWidth);
            m_instrImage->setMaximumWidth(scaledWidth);
        }
    }
}
//方案管理
void FMFD::openSchemeManagement() {
    // ✅ 重要：在打开对话框前，将当前的所有配置同步到 m_currentScheme
    m_currentScheme.sweepConfig = m_currentConfig;
    m_currentScheme.saConfig = m_currentSaConfig;
    m_currentScheme.sgConfig = m_currentSgConfig;

    m_schemeDialog->setCurrentScheme(m_currentScheme);
    if (m_schemeDialog->exec() == QDialog::Accepted) {
        onSchemeLoaded();
    }
}

void FMFD::onSchemeLoaded() {
    m_currentScheme = m_schemeDialog->getSelectedScheme();

    // ✅ 同步到各个独立的配置变量
    m_currentConfig = m_currentScheme.sweepConfig;
    m_currentSaConfig = m_currentScheme.saConfig;
    m_currentSgConfig = m_currentScheme.sgConfig;

    // 同步到各配置对话框
    m_configDialog->setConfig(m_currentScheme.sweepConfig);
    m_saConfigDialog->setConfig(m_currentScheme.saConfig);
    m_sgConfigDialog->setConfig(m_currentScheme.sgConfig);

    QString displayName = !m_currentScheme.name.isEmpty() ? m_currentScheme.name : m_currentScheme.schemeName;
    m_diagText->append("[方案] 已加载方案: " + displayName);
}

void FMFD::saveSchemeDialog()
{
    bool ok;
    QString schemeName = QInputDialog::getText(this, "保存方案", "方案名称:",
        QLineEdit::Normal, "", &ok);
    if (ok && !schemeName.isEmpty()) {
        if (m_schemeManager->schemeExists(schemeName)) {
            if (QMessageBox::question(this, "确认", "方案 " + schemeName + " 已存在，是否覆盖?",
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
                return;
            }
        }

        TestScheme scheme;
        scheme.name = schemeName;
        scheme.sweepConfig = m_currentConfig;
        scheme.saConfig = m_currentSaConfig;
        scheme.sgConfig = m_currentSgConfig;

        if (m_schemeManager->saveScheme(scheme)) {
            QMessageBox::information(this, "成功", "方案已保存: " + schemeName);
            onSchemeSaved();
        }
        else {
            QMessageBox::warning(this, "失败", "方案保存失败");
        }
    }
}

void FMFD::loadSchemeDialog()
{
    QStringList schemes = m_schemeManager->listSchemes();
    if (schemes.isEmpty()) {
        QMessageBox::information(this, "信息", "没有可用的方案");
        return;
    }

    bool ok;
    QString schemeName = QInputDialog::getItem(this, "加载方案", "选择方案:",
        schemes, 0, false, &ok);
    if (ok && !schemeName.isEmpty()) {
        TestScheme scheme;
        if (m_schemeManager->loadScheme(schemeName, scheme)) {
            m_currentConfig = scheme.sweepConfig;
            m_currentSaConfig = scheme.saConfig;
            m_currentSgConfig = scheme.sgConfig;

            // 更新 UI
            if (m_configDialog) {
                m_configDialog->setConfig(m_currentConfig);
            }
            if (m_saConfigDialog) {
                m_saConfigDialog->setConfig(m_currentSaConfig);
            }
            if (m_sgConfigDialog) {
                m_sgConfigDialog->setConfig(m_currentSgConfig);
            }

            QMessageBox::information(this, "成功", "方案已加载: " + schemeName);
            onSchemeLoaded();
        }
        else {
            QMessageBox::warning(this, "失败", "方案加载失败");
        }
    }
}

void FMFD::deleteSchemeDialog()
{
    QStringList schemes = m_schemeManager->listSchemes();
    if (schemes.isEmpty()) {
        QMessageBox::information(this, "信息", "没有可用的方案");
        return;
    }

    bool ok;
    QString schemeName = QInputDialog::getItem(this, "删除方案", "选择方案:",
        schemes, 0, false, &ok);
    if (ok && !schemeName.isEmpty()) {
        if (QMessageBox::question(this, "确认", "确定要删除方案 " + schemeName + " 吗?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            if (m_schemeManager->deleteScheme(schemeName)) {
                QMessageBox::information(this, "成功", "方案已删除");
            }
            else {
                QMessageBox::warning(this, "失败", "方案删除失败");
            }
        }
    }
}

void FMFD::onSchemeSaved()
{
    // 可以在这里添加保存后的处理逻辑
    qDebug() << "Scheme saved callback";
}

// ✅ 新增：处理方案管理对话框的配置编辑请求
void FMFD::onSchemeEditFrequencySweep()
{
    if (!m_configDialog) {
        m_configDialog = new FrequencySweepConfigDialog(this);
        connect(m_configDialog, &QDialog::accepted, this, &FMFD::onConfigDialogAccepted);
    }

    // 获取当前方案并设置配置到对话框
    TestScheme currentScheme = m_schemeDialog->getSelectedScheme();
    m_configDialog->setConfig(currentScheme.sweepConfig);

    if (m_configDialog->exec() == QDialog::Accepted) {
        // 更新方案配置
        currentScheme.sweepConfig = m_configDialog->getConfig();
        // 同步回对话框
        m_schemeDialog->setCurrentScheme(currentScheme);
        m_diagText->append("[方案编辑] 频率扫描配置已更新");
    }
}

void FMFD::onSchemeEditSpectrumAnalyzer()
{
    if (!m_saConfigDialog) {
        m_saConfigDialog = new SpectrumAnalyzerConfigDialog(this);
        connect(m_saConfigDialog, &QDialog::accepted, this, &FMFD::onSaConfigAccepted);
    }

    // 获取当前方案并设置配置到对话框
    TestScheme currentScheme = m_schemeDialog->getSelectedScheme();
    m_saConfigDialog->setConfig(currentScheme.saConfig);

    if (m_saConfigDialog->exec() == QDialog::Accepted) {
        // 更新方案配置
        currentScheme.saConfig = m_saConfigDialog->getConfig();
        // 同步回对话框
        m_schemeDialog->setCurrentScheme(currentScheme);
        m_diagText->append("[方案编辑] 频谱分析仪配置已更新");
    }
}

void FMFD::onSchemeEditSignalGenerator()
{
    if (!m_sgConfigDialog) {
        m_sgConfigDialog = new SignalGeneratorConfigDialog(this);
        connect(m_sgConfigDialog, &QDialog::accepted, this, &FMFD::onSgConfigAccepted);
    }

    // 获取当前方案并设置配置到对话框
    TestScheme currentScheme = m_schemeDialog->getSelectedScheme();
    m_sgConfigDialog->setConfig(currentScheme.sgConfig);

    if (m_sgConfigDialog->exec() == QDialog::Accepted) {
        // 更新方案配置
        currentScheme.sgConfig = m_sgConfigDialog->getConfig();
        // 同步回对话框
        m_schemeDialog->setCurrentScheme(currentScheme);
        m_diagText->append("[方案编辑] 信号发生器配置已更新");
    }
}

//使用说明
void FMFD::openUsageDialog()
{
    UsageDialog dialog(this);
    dialog.exec();
}

// ============ BRB诊断Python集成实现 ============

void FMFD::configureBRBPaths()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("BRB Python路径配置"));
    dialog.setMinimumWidth(600);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Python解释器路径
    QHBoxLayout* pythonLayout = new QHBoxLayout();
    pythonLayout->addWidget(new QLabel(tr("Python解释器路径:")));
    QLineEdit* pythonEdit = new QLineEdit(m_brbPythonPath, &dialog);
    pythonLayout->addWidget(pythonEdit);
    QPushButton* browsePythonBtn = new QPushButton(tr("浏览..."), &dialog);
    pythonLayout->addWidget(browsePythonBtn);
    layout->addLayout(pythonLayout);

    // Python脚本路径
    QHBoxLayout* scriptLayout = new QHBoxLayout();
    scriptLayout->addWidget(new QLabel(tr("Python脚本路径:")));
    QLineEdit* scriptEdit = new QLineEdit(m_brbScriptPath, &dialog);
    scriptLayout->addWidget(scriptEdit);
    QPushButton* browseScriptBtn = new QPushButton(tr("浏览..."), &dialog);
    scriptLayout->addWidget(browseScriptBtn);
    layout->addLayout(scriptLayout);

    // EXE路径
    QHBoxLayout* exeLayout = new QHBoxLayout();
    exeLayout->addWidget(new QLabel(tr("打包EXE路径:")));
    QLineEdit* exeEdit = new QLineEdit(m_brbExePath, &dialog);
    exeLayout->addWidget(exeEdit);
    QPushButton* browseExeBtn = new QPushButton(tr("浏览..."), &dialog);
    exeLayout->addWidget(browseExeBtn);
    layout->addLayout(exeLayout);

    // 说明文本
    QTextEdit* helpText = new QTextEdit(&dialog);
    helpText->setReadOnly(true);
    helpText->setMaximumHeight(150);
    helpText->setPlainText(
        tr("说明：\n"
            "1. Python解释器路径：用于运行Python脚本\n"
            "   - 可以是 python、python3 或虚拟环境中的python.exe完整路径\n"
            "   - 如遇到NumPy/Pandas版本兼容性问题，请指定虚拟环境的Python路径\n"
            "   - 示例：E:\\Anaconda3\\envs\\tf_12\\python.exe\n"
            "2. Python脚本路径：brb_diagnosis_cli.py的完整路径\n"
            "3. 打包EXE路径：Python脚本打包后的exe文件路径\n"
            "   （默认：程序目录/x64/Release/BRB/brb_diagnosis.exe）\n"
            "系统会优先使用EXE，如果不存在则使用Python脚本。\n"
            "配置将自动保存，下次启动时会自动加载。")
    );
    layout->addWidget(helpText);

    // 按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* okBtn = new QPushButton(tr("确定"), &dialog);
    QPushButton* cancelBtn = new QPushButton(tr("取消"), &dialog);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    // 浏览按钮连接
    connect(browsePythonBtn, &QPushButton::clicked, [&]() {
        QString path = QFileDialog::getOpenFileName(&dialog, tr("选择Python解释器"),
            pythonEdit->text(), tr("可执行文件 (*.exe);;所有文件 (*)"));
        if (!path.isEmpty()) pythonEdit->setText(path);
        });

    connect(browseScriptBtn, &QPushButton::clicked, [&]() {
        QString path = QFileDialog::getOpenFileName(&dialog, tr("选择Python脚本"),
            scriptEdit->text(), tr("Python文件 (*.py);;所有文件 (*)"));
        if (!path.isEmpty()) scriptEdit->setText(path);
        });

    connect(browseExeBtn, &QPushButton::clicked, [&]() {
        QString path = QFileDialog::getOpenFileName(&dialog, tr("选择BRB诊断EXE"),
            exeEdit->text(), tr("可执行文件 (*.exe);;所有文件 (*)"));
        if (!path.isEmpty()) exeEdit->setText(path);
        });

    connect(okBtn, &QPushButton::clicked, [&]() {
        m_brbPythonPath = pythonEdit->text();
        m_brbScriptPath = scriptEdit->text();
        m_brbExePath = exeEdit->text();

        // 保存配置到QSettings
        QSettings settings("FMFD", "FMFD-Software");
        settings.setValue("BRB/PythonPath", m_brbPythonPath);
        settings.setValue("BRB/ScriptPath", m_brbScriptPath);
        settings.setValue("BRB/ExePath", m_brbExePath);

        dialog.accept();
        });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void FMFD::runBRBDiagnosis()
{
    if (!m_brbProc) {
        m_diagText->append(tr("[BRB诊断错误] BRB进程未初始化"));
        return;
    }

    if (m_brbProc->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, tr("警告"), tr("BRB诊断进程正在运行中，请等待完成。"));
        return;
    }

    // 选择输入文件
    QString inputFile = QFileDialog::getOpenFileName(this,
        tr("选择频响数据文件"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("CSV文件 (*.csv);;所有文件 (*)"));

    if (inputFile.isEmpty()) {
        return;
    }

    // 保存当前输入文件路径（用于 Ground Truth 验收）
    m_currentInputCsvPath = inputFile;

    // 加载CSV文件数据用于频响曲线显示
    if (loadCsvForFrequencyResponse(inputFile)) {
        // 自动切换到频响曲线显示
        QAbstractButton* freqRespButton = m_imageButtonGroup->button(2);
        if (freqRespButton) {
            freqRespButton->setChecked(true);
            updateFrequencyResponsePlot();
            m_diagText->append(tr("[频响曲线] 已从BRB诊断CSV文件加载数据并显示"));
        }
    }

    // 创建带时间戳的运行目录
    m_brbRunDir = createBrbRunDir();

    // 设置输出文件路径（在运行目录中）
    QString outputFile = m_brbRunDir + "/brb_diagnosis_result.json";

    m_diagText->append(tr("[BRB诊断] 开始诊断..."));
    m_diagText->append(tr("[BRB诊断] 输入文件: %1").arg(inputFile));
    m_diagText->append(tr("[BRB诊断] 输出目录: %1").arg(m_brbRunDir));
    m_diagText->append(tr("[BRB诊断] 输出文件: %1").arg(outputFile));

    // 优先使用exe，如果不存在则使用Python脚本
    bool useExe = QFileInfo::exists(m_brbExePath);

    QString program;
    QStringList arguments;

    if (useExe) {
        program = m_brbExePath;
        arguments << "--input" << inputFile << "--output" << outputFile << "--verbose";
        m_diagText->append(tr("[BRB诊断] 使用EXE: %1").arg(m_brbExePath));
    }
    else {
        program = m_brbPythonPath;
        arguments << m_brbScriptPath << "--input" << inputFile << "--output" << outputFile << "--verbose";
        m_diagText->append(tr("[BRB诊断] 使用Python脚本: %1").arg(m_brbScriptPath));
        m_diagText->append(tr("[BRB诊断] Python解释器: %1").arg(m_brbPythonPath));

        if (!QFileInfo::exists(m_brbScriptPath)) {
            QMessageBox::critical(this, tr("错误"),
                tr("Python脚本不存在: %1\n请在配置中设置正确的路径。").arg(m_brbScriptPath));
            return;
        }

        // 提示用户可以配置Python路径
        if (m_brbPythonPath == "python" || m_brbPythonPath == "python3") {
            m_diagText->append(tr("[提示] 当前使用系统默认Python，如遇到环境问题请通过'配置'->'BRB Python路径配置'设置虚拟环境路径"));
        }
    }

    m_diagText->append(tr("[BRB诊断] 执行命令: %1 %2").arg(program, arguments.join(" ")));

    // 设置工作目录为应用程序目录（与viz-cli.exe相同的方式）
    QString appDir = QCoreApplication::applicationDirPath();
    m_brbProc->setWorkingDirectory(appDir);

    // 继承系统环境变量，确保使用配置好的Python环境
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // 设置Python输出编码为UTF-8，避免中文乱码
    env.insert("PYTHONIOENCODING", "utf-8");
    m_brbProc->setProcessEnvironment(env);

    m_brbProc->setProgram(program);
    m_brbProc->setArguments(arguments);
    m_brbProc->start();

    constexpr int BRB_PROCESS_START_TIMEOUT_MS = 3000;
    if (!m_brbProc->waitForStarted(BRB_PROCESS_START_TIMEOUT_MS)) {
        m_diagText->append(tr("[BRB诊断错误] 进程启动失败: %1").arg(m_brbProc->errorString()));
        QMessageBox::critical(this, tr("错误"),
            tr("无法启动BRB诊断进程。\n错误: %1").arg(m_brbProc->errorString()));
    }
}

void FMFD::onBRBDiagnosisReadyRead()
{
    if (!m_brbProc) return;

    QByteArray stdOut = m_brbProc->readAllStandardOutput();
    QByteArray stdErr = m_brbProc->readAllStandardError();

    if (!stdOut.isEmpty()) {
        QString output = QString::fromUtf8(stdOut);
        m_diagText->append(tr("[BRB输出] %1").arg(output));
    }

    if (!stdErr.isEmpty()) {
        QString error = QString::fromUtf8(stdErr);
        m_diagText->append(tr("[BRB错误] %1").arg(error));

        // 检测常见的Python环境问题
        if (error.contains("AttributeError") && error.contains("numpy") && error.contains("bool")) {
            m_diagText->append(tr("\n[提示] 检测到NumPy版本兼容性问题！"));
            m_diagText->append(tr("[提示] 这通常是因为使用了不兼容版本的NumPy和Pandas。"));
            m_diagText->append(tr("[提示] 解决方法："));
            m_diagText->append(tr("[提示] 1. 在菜单栏选择 '配置' -> 'BRB Python路径配置'"));
            m_diagText->append(tr("[提示] 2. 将'Python解释器路径'设置为兼容环境的Python可执行文件"));
            m_diagText->append(tr("[提示] 3. 例如：E:\\Anaconda3\\envs\\tf_12\\python.exe"));
        }
        else if (error.contains("ModuleNotFoundError") || error.contains("ImportError")) {
            m_diagText->append(tr("\n[提示] 检测到Python模块导入错误！"));
            m_diagText->append(tr("[提示] 请确保所需的Python包已安装在配置的Python环境中。"));
        }
    }
}

void FMFD::onBRBDiagnosisFinished(int exitCode, QProcess::ExitStatus status)
{
    // 保存 stdout/stderr 到日志文件
    if (!m_brbRunDir.isEmpty()) {
        QString stdoutLogPath = m_brbRunDir + "/python_stdout.log";
        QFile logFile(stdoutLogPath);
        if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&logFile);
            out << "=== BRB Diagnosis Python Output ===" << Qt::endl;
            out << "Exit Code: " << exitCode << Qt::endl;
            out << "Status: " << (status == QProcess::NormalExit ? "Normal" : "Crashed") << Qt::endl;
            out << Qt::endl << "=== Log from UI ===" << Qt::endl;
            // 这里简化处理，实际 stdout/stderr 已经在 onBRBDiagnosisReadyRead 中处理
            logFile.close();
            m_diagText->append(tr("[BRB诊断] Python日志已保存到: %1").arg(stdoutLogPath));
        }
    }

    m_diagText->append(tr("[BRB诊断] 进程结束，退出码: %1").arg(exitCode));

    // 检查 exitCode
    if (exitCode != 0) {
        m_diagText->append(tr("[BRB诊断错误] Python进程非正常退出（exitCode=%1）").arg(exitCode));
        m_diagText->append(tr("[BRB诊断错误] 请检查上方的错误输出以定位问题"));
        QMessageBox::warning(this, tr("诊断失败"),
            tr("BRB诊断进程异常退出（退出码: %1）\n请查看日志获取详细信息。").arg(exitCode));
        return;
    }

    // 读取并显示结果
    QString outputFile = m_brbRunDir + "/brb_diagnosis_result.json";

    // 检查文件是否存在且可读
    if (!QFileInfo::exists(outputFile)) {
        m_diagText->append(tr("[BRB诊断错误] 结果文件不存在: %1").arg(outputFile));
        QMessageBox::warning(this, tr("诊断失败"), tr("BRB诊断结果文件不存在。"));
        return;
    }

    // 使用 BRBEngine::loadDiagnosisResult 解析 JSON
    QString errorMsg;
    DiagnosisResult diagResult = BRBEngine::loadDiagnosisResult(outputFile, &errorMsg);
    
    if (!errorMsg.isEmpty()) {
        m_diagText->append(tr("[BRB诊断错误] %1").arg(errorMsg));
        QMessageBox::warning(this, tr("诊断失败"), errorMsg);
        return;
    }

    // 尝试加载 Ground Truth（仅对 sim_* 文件）
    QString sampleId = extractSampleId(m_currentInputCsvPath);
    if (!sampleId.isEmpty()) {
        QString labelsJsonPath = QCoreApplication::applicationDirPath() + "/Output/sim_spectrum/labels.json";
        QString gtSystemFault, gtModule;
        if (BRBEngine::loadGroundTruth(labelsJsonPath, sampleId, gtSystemFault, gtModule)) {
            diagResult.hasGroundTruth = true;
            diagResult.gtSystemFaultClass = gtSystemFault;
            diagResult.gtModule = gtModule;
            
            // 验收匹配判断
            // 系统故障类型映射：将中文 predicted_class 映射回英文类型
            QString predictedFaultType;
            if (diagResult.systemDiagnosis.predictedClass == QStringLiteral("正常")) {
                predictedFaultType = QStringLiteral("normal");
            } else if (diagResult.systemDiagnosis.predictedClass == QStringLiteral("幅度失准")) {
                predictedFaultType = QStringLiteral("amp_error");
            } else if (diagResult.systemDiagnosis.predictedClass == QStringLiteral("频率失准")) {
                predictedFaultType = QStringLiteral("freq_error");
            } else if (diagResult.systemDiagnosis.predictedClass == QStringLiteral("参考电平失准")) {
                predictedFaultType = QStringLiteral("ref_error");
            }
            
            diagResult.matchResult = (predictedFaultType == gtSystemFault);
        }
    }

    // 更新 UI
    updateDiagnosisUI(diagResult);

    m_diagText->append(tr("[BRB诊断] 完整结果已保存到: %1").arg(outputFile));

    QMessageBox::information(this, tr("诊断完成"),
        tr("BRB诊断完成！\n详细结果已保存到:\n%1").arg(outputFile));
}

// ============ BRB诊断结果UI更新函数 ============

QString FMFD::createBrbRunDir()
{
    // 创建带时间戳的运行目录
    QString baseDir = QCoreApplication::applicationDirPath() + "/Output/ui_runs";
    QDir dir(baseDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString runDir = baseDir + "/" + timestamp;
    QDir(runDir).mkpath(".");
    
    return runDir;
}

QString FMFD::extractSampleId(const QString& csvFileName)
{
    // 从文件名提取 sample_id（如 sim_00009.csv -> sim_00009）
    QFileInfo fi(csvFileName);
    QString baseName = fi.baseName();  // 不含扩展名
    
    // 检查是否以 sim_ 开头
    if (baseName.startsWith("sim_")) {
        return baseName;
    }
    return QString();  // 非 sim_* 文件返回空
}

void FMFD::updateDiagnosisUI(const DiagnosisResult& result)
{
    m_diagText->append(tr("==================== BRB诊断结果 / BRB Diagnosis Result ===================="));
    
    // 显示基本信息
    m_diagText->append(tr("\n【基本信息 / Basic Info】"));
    m_diagText->append(tr("  输入文件 (Input File): %1").arg(result.inputFile));
    m_diagText->append(tr("  数据点数 (Data Points): %1").arg(result.dataPoints));
    m_diagText->append(tr("  频率范围 (Frequency Range): %1 - %2 Hz")
        .arg(result.frequencyRange.min, 0, 'e', 2)
        .arg(result.frequencyRange.max, 0, 'e', 2));

    // 更新特征表格
    if (!result.features.isEmpty()) {
        refreshFeatureTable(result.features);
        m_diagText->append(tr("\n【测量特征 / Features】已更新到特征表格（共 %1 项）").arg(result.features.size()));
    }

    // 更新系统级诊断 UI
    updateSystemDiagnosisUI(result.systemDiagnosis);

    // 更新模块级诊断 UI
    if (!result.moduleDiagnosis.isEmpty()) {
        updateModuleDiagnosisUI(result.moduleDiagnosis);
    }

    // 更新 Ground Truth 验收 UI
    updateGroundTruthUI(result);

    m_diagText->append(tr("===================================================="));
}

void FMFD::updateSystemDiagnosisUI(const SystemDiagnosis& sysDiag)
{
    m_diagText->append(tr("\n【系统级诊断 / System-Level Diagnosis】"));

    // 系统故障类型映射（中文->英文）
    static const QMap<QString, QString> faultTypeMap = {
        {QStringLiteral("正常"), QStringLiteral("Normal")},
        {QStringLiteral("幅度失准"), QStringLiteral("Amplitude Error")},
        {QStringLiteral("频率失准"), QStringLiteral("Frequency Error")},
        {QStringLiteral("参考电平失准"), QStringLiteral("Reference Level Error")}
    };
    
    // predicted_class - 显示中英文
    QString englishClass = faultTypeMap.value(sysDiag.predictedClass, sysDiag.predictedClass);
    m_diagText->append(tr("  预测类别 (Predicted Class): %1 (%2)")
        .arg(sysDiag.predictedClass, englishClass));
    
    // max_prob - 显示百分比
    m_diagText->append(tr("  最大概率 (Max Probability): %1%").arg(sysDiag.maxProb * 100, 0, 'f', 2));
    
    // is_normal - 显示 "正常/异常" 中英文
    QString normalStatusCN = sysDiag.isNormal ? tr("正常") : tr("异常");
    QString normalStatusEN = sysDiag.isNormal ? "Normal" : "Abnormal";
    m_diagText->append(tr("  系统状态 (System Status): %1 (%2)").arg(normalStatusCN, normalStatusEN));

    // 显示四类概率分布
    QString topClass = sysDiag.predictedClass;
    double topProb = sysDiag.maxProb;
    if (!sysDiag.probabilities.isEmpty()) {
        m_diagText->append(tr("  概率分布 (Probability Distribution):"));
        topProb = -1.0;
        for (auto it = sysDiag.probabilities.constBegin(); it != sysDiag.probabilities.constEnd(); ++it) {
            QString englishName = faultTypeMap.value(it.key(), it.key());
            m_diagText->append(tr("    - %1 (%2): %3%")
                .arg(it.key())
                .arg(englishName)
                .arg(it.value() * 100, 0, 'f', 2));
            if (it.value() > topProb) {
                topProb = it.value();
                topClass = it.key();
            }
        }
    }

    // 根据最高概率类别自动切换结构图
    static const QMap<QString, int> classToVizMode = {
        {QStringLiteral("正常"), 0},
        {QStringLiteral("频率失准"), 1},
        {QStringLiteral("幅度失准"), 2},
        {QStringLiteral("参考电平失准"), 4}
    };
    if (classToVizMode.contains(topClass)) {
        requestPythonVisualization(classToVizMode.value(topClass));
    }

    // 日志区打印关键信息（便于调试）
    m_diagText->append(tr("\n[Log/日志] predicted_class=%1, max_prob=%2%, is_normal=%3, top_class=%4")
        .arg(sysDiag.predictedClass)
        .arg(sysDiag.maxProb * 100, 0, 'f', 2)
        .arg(sysDiag.isNormal ? "true" : "false")
        .arg(topClass));
}

void FMFD::updateModuleDiagnosisUI(const QMap<QString, double>& moduleDiag)
{
    m_diagText->append(tr("\n【模块级诊断 TOP10 / Module-Level Diagnosis TOP10】"));

    // 中英文模块名映射
    static const QMap<QString, QString> moduleNameMap = {
        {QStringLiteral("衰减器"), QStringLiteral("Attenuator")},
        {QStringLiteral("前置放大器"), QStringLiteral("Preamp")},
        {QStringLiteral("低频段前置低通滤波器"), QStringLiteral("Lowband_LPF")},
        {QStringLiteral("低频段第一混频器"), QStringLiteral("Lowband_Mixer1")},
        {QStringLiteral("低频段滤波器1"), QStringLiteral("Lowband_Filter1")},
        {QStringLiteral("低频段第二混频器"), QStringLiteral("Lowband_Mixer2")},
        {QStringLiteral("低频段滤波器2"), QStringLiteral("Lowband_Filter2")},
        {QStringLiteral("高频段YTF滤波器"), QStringLiteral("Highband_YTF")},
        {QStringLiteral("高频段混频器"), QStringLiteral("Highband_Mixer")},
        {QStringLiteral("时钟振荡器"), QStringLiteral("Clock_Oscillator")},
        {QStringLiteral("时钟合成与同步网络"), QStringLiteral("Clock_Synth")},
        {QStringLiteral("本振源（谐波发生器）"), QStringLiteral("LO_Source")},
        {QStringLiteral("本振混频组件"), QStringLiteral("LO_Mixer")},
        {QStringLiteral("校准源"), QStringLiteral("Cal_Source")},
        {QStringLiteral("存储器"), QStringLiteral("Cal_Memory")},
        {QStringLiteral("校准信号开关"), QStringLiteral("Cal_Switch")},
        {QStringLiteral("中频放大器"), QStringLiteral("IF_Amplifier")},
        {QStringLiteral("ADC"), QStringLiteral("ADC")},
        {QStringLiteral("数字RBW"), QStringLiteral("FPGA_DSP")},
        {QStringLiteral("数字放大器"), QStringLiteral("Digital_Amp")},
        {QStringLiteral("数字检波器"), QStringLiteral("Digital_Detector")},
        {QStringLiteral("VBW滤波器"), QStringLiteral("VBW_Filter")},
        {QStringLiteral("电源模块"), QStringLiteral("Power_Module")},
        {QStringLiteral("未定义/其他"), QStringLiteral("Undefined/Other")}
    };

    // 转换为 list 并按概率降序排序
    QList<QPair<QString, double>> sortedModules;
    for (auto it = moduleDiag.constBegin(); it != moduleDiag.constEnd(); ++it) {
        sortedModules.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedModules.begin(), sortedModules.end(),
        [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
            return a.second > b.second;
        });

    // 显示 TOP10
    int topCount = qMin(10, sortedModules.size());
    for (int i = 0; i < topCount; ++i) {
        const QString& moduleName = sortedModules[i].first;
        double prob = sortedModules[i].second;
        
        // 获取英文模块名（如果有映射）
        QString englishName = moduleNameMap.value(moduleName, moduleName);
        QString displayName = (moduleName != englishName) 
            ? QString("%1 (%2)").arg(moduleName, englishName)
            : moduleName;
        
        // 检查是否是前置放大器且概率为0（前放关闭的情况）
        bool isPreampDisabled = (moduleName.contains("Preamp") || moduleName.contains(QStringLiteral("前置放大器"))) 
                                && prob < 0.001;
        
        if (isPreampDisabled) {
            m_diagText->append(tr("  %1. %2: %3% [Disabled/Off 禁用]")
                .arg(i + 1)
                .arg(displayName)
                .arg(prob * 100, 0, 'f', 2));
        } else {
            m_diagText->append(tr("  %1. %2: %3%")
                .arg(i + 1)
                .arg(displayName)
                .arg(prob * 100, 0, 'f', 2));
        }
    }

    // 清空现有的进度条布局并重建
    if (m_diagScrollLayout) {
        // 完全安全的删除方式：使用deleteLater递归删除所有子widget和layout
        while (m_diagScrollLayout->count() > 0) {
            QLayoutItem* item = m_diagScrollLayout->takeAt(0);
            if (item) {
                if (item->widget()) {
                    item->widget()->deleteLater();
                }
                else if (item->layout()) {
                    // 递归删除布局中的所有widget
                    while (item->layout()->count() > 0) {
                        QLayoutItem* subItem = item->layout()->takeAt(0);
                        if (subItem) {
                            if (subItem->widget()) {
                                subItem->widget()->deleteLater();
                            }
                            // subItem will be deleted when its parent layout is deleted
                        }
                    }
                    // Layout item will be deleted with parent, use deleteLater for safety
                }
                // Don't delete item immediately when widgets use deleteLater
                // The QLayoutItem will be cleaned up when layout is repopulated
            }
        }

        // 清空m_moduleBars映射
        m_moduleBars.clear();

        // 重新创建进度条（按排序后的顺序）
        for (int i = 0; i < topCount; ++i) {
            const QString& moduleName = sortedModules[i].first;
            double prob = sortedModules[i].second;
            
            // 获取英文模块名（如果有映射）用于进度条显示
            QString englishName = moduleNameMap.value(moduleName, moduleName);
            QString displayName = (moduleName != englishName) 
                ? QString("%1 (%2)").arg(moduleName, englishName)
                : moduleName;
            
            // 检查是否是前置放大器且概率为0
            bool isPreampDisabled = (moduleName.contains("Preamp") || moduleName.contains(QStringLiteral("前置放大器"))) 
                                    && prob < 0.001;

            // 进度条标签显示百分比
            QString labelText = QString("%1: %2%").arg(displayName).arg(prob * 100, 0, 'f', 1);
            QLabel* lbl = new QLabel(labelText, m_diagScrollContent);
            QProgressBar* bar = new QProgressBar(m_diagScrollContent);
            bar->setRange(0, 100);
            int percentage = static_cast<int>(prob * 100);
            bar->setValue(percentage);

            // 根据概率和状态设置颜色
            if (isPreampDisabled) {
                // 前放关闭：灰色显示
                bar->setStyleSheet("QProgressBar::chunk { background-color: #cccccc; }");
                lbl->setStyleSheet("color: #888888;");
            } else if (prob > 0.1) {
                // 高概率用红色样式
                bar->setStyleSheet("QProgressBar::chunk { background-color: #ff6666; }");
            } else {
                // 低概率用默认样式
                bar->setStyleSheet("");
            }

            m_moduleBars[moduleName] = bar;

            QHBoxLayout* row = new QHBoxLayout();
            row->addWidget(lbl);
            row->addWidget(bar);
            m_diagScrollLayout->addLayout(row);
        }

        m_diagScrollLayout->addStretch();
    }

    // 更新图形高亮显示（如果有图形项）
    for (auto it = m_graphicsItems.constBegin(); it != m_graphicsItems.constEnd(); ++it) {
        double prob = moduleDiag.value(it.key(), 0.0);
        QColor color;
        if (prob > 0.1) {
            // 高概率用红色
            int intensity = static_cast<int>(prob * 255);
            color = QColor(255, 255 - intensity, 255 - intensity);
        }
        else {
            // 低概率用浅色
            color = QColor(240, 240, 240);
        }
        it.value()->setBrush(QBrush(color));
    }

    // 触发 BRB 引擎的诊断完成信号（用于结构图可视化）
    QMap<QString, double> moduleProbabilities;
    for (const auto& pair : sortedModules) {
        moduleProbabilities[pair.first] = pair.second;
    }
    
    // 更新结构图可视化
    requestPythonVisualization(4);  // mode=4 触发symptom模式更新图形

    m_diagText->append(tr("[BRB Diagnosis] Module diagnosis results visualized / 已将模块诊断结果可视化到BRB诊断区域"));
    
    // 打印 TopK 模块到日志
    m_diagText->append(tr("\n[Log/日志] TopK Modules / TopK模块:"));
    for (int i = 0; i < qMin(5, topCount); ++i) {
        QString englishName = moduleNameMap.value(sortedModules[i].first, sortedModules[i].first);
        m_diagText->append(tr("  - %1 (%2): %3%")
            .arg(sortedModules[i].first)
            .arg(englishName)
            .arg(sortedModules[i].second * 100, 0, 'f', 2));
    }
}

void FMFD::updateGroundTruthUI(const DiagnosisResult& result)
{
    m_diagText->append(tr("\n【Ground Truth 验收 / Validation】"));
    
    if (!result.hasGroundTruth) {
        m_diagText->append(tr("  GT: N/A (Not a sim_* simulation file / 非 sim_* 仿真文件)"));
        return;
    }
    
    // 显示 Ground Truth
    m_diagText->append(tr("  GT System Fault Type / 系统故障类型: %1").arg(result.gtSystemFaultClass));
    m_diagText->append(tr("  GT Fault Module / 故障模块: %1").arg(result.gtModule));
    
    // 显示 Match 结果
    QString matchStr = result.matchResult ? tr("OK ✓") : tr("NG ✗");
    m_diagText->append(tr("  Match / 匹配: %1").arg(matchStr));
    
    if (!result.matchResult) {
        m_diagText->append(tr("  [Warning/警告] Prediction does not match Ground Truth / 预测结果与 Ground Truth 不匹配！"));
    }
}

// ============ 频响曲线相关函数实现 ============

// 频响曲线绘图常量
namespace FreqResponsePlotConstants {
    // 数据处理常量
    constexpr double MIN_FREQ_RANGE = 1e6;      // 最小频率范围，防止除零
    constexpr double MIN_AMP_RANGE = 10.0;      // 最小幅度范围，防止除零
    constexpr double EPSILON = 1e-6;            // 浮点比较阈值
    constexpr double RANGE_MARGIN_RATIO = 0.1;  // 数据范围边距比例
    constexpr double FIXED_MIN_AMP = -10.5;     // 固定Y轴下限（dBm）
    constexpr double FIXED_MAX_AMP = -9.5;      // 固定Y轴上限（dBm）
    
    // 绘图布局边距
    constexpr int MARGIN_LEFT = 60;
    constexpr int MARGIN_RIGHT = 20;
    constexpr int MARGIN_TOP = 30;
    constexpr int MARGIN_BOTTOM = 50;
    
    // 网格线数量
    constexpr int GRID_COUNT_X = 10;
    constexpr int GRID_COUNT_Y = 8;
    
    // 数据点标记阈值
    constexpr int MAX_POINTS_FOR_MARKERS = 100;
}

void FMFD::clearFrequencyResponseData()
{
    m_frequencyResponseData.clear();
}

// 辅助函数：解析CSV行数据
static bool parseCsvLineForFrequencyResponse(const QStringList& parts, double& freqHz, double& amplitude)
{
    if (parts.size() >= 3) {
        // CSV格式: Frequency(Hz), Frequency(MHz), Amplitude(dBm)
        bool okFreq = false, okAmp = false;
        freqHz = parts[0].trimmed().toDouble(&okFreq);
        amplitude = parts[2].trimmed().toDouble(&okAmp);
        return okFreq && okAmp;
    }
    else if (parts.size() >= 2) {
        // CSV格式: Frequency(Hz), Amplitude(dBm)
        bool okFreq = false, okAmp = false;
        freqHz = parts[0].trimmed().toDouble(&okFreq);
        amplitude = parts[1].trimmed().toDouble(&okAmp);
        return okFreq && okAmp;
    }
    return false;
}

bool FMFD::loadCsvForFrequencyResponse(const QString& csvFilePath)
{
    QFile file(csvFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_diagText->append(tr("[频响曲线] 无法打开CSV文件: %1").arg(csvFilePath));
        return false;
    }

    m_frequencyResponseData.clear();
    QTextStream in(&file);
    bool isFirstLine = true;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 跳过标题行
        if (isFirstLine) {
            isFirstLine = false;
            if (line.contains("Frequency") || line.contains("frequency") || line.contains("Hz")) {
                continue;
            }
        }

        QStringList parts = line.split(',');
        double freqHz = 0.0, amplitude = 0.0;
        if (parseCsvLineForFrequencyResponse(parts, freqHz, amplitude)) {
            m_frequencyResponseData.append(qMakePair(freqHz, amplitude));
        }
    }

    file.close();
    m_lastBrbCsvFile = csvFilePath;
    m_diagText->append(tr("[频响曲线] 已从CSV文件加载 %1 个数据点").arg(m_frequencyResponseData.size()));
    return !m_frequencyResponseData.isEmpty();
}

void FMFD::updateFrequencyResponsePlot()
{
    using namespace FreqResponsePlotConstants;
    
    if (m_frequencyResponseData.isEmpty()) {
        // 显示提示信息
        int fixedHeight = int(this->height() * 0.4);
        int plotWidth = int(fixedHeight * 2);  // 宽高比约2:1
        
        QPixmap plotPixmap(plotWidth, fixedHeight);
        plotPixmap.fill(Qt::white);
        
        QPainter painter(&plotPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 绘制边框
        painter.setPen(QPen(Qt::gray, 1));
        painter.drawRect(0, 0, plotWidth - 1, fixedHeight - 1);
        
        // 绘制提示文字
        painter.setPen(Qt::darkGray);
        QFont font = painter.font();
        font.setPointSize(12);
        painter.setFont(font);
        painter.drawText(plotPixmap.rect(), Qt::AlignCenter, tr("暂无频响数据\n请先进行一键测试或运行BRB诊断加载CSV文件"));
        
        painter.end();
        
        m_instrImage->setPixmap(plotPixmap);
        m_instrImage->setMinimumHeight(fixedHeight);
        m_instrImage->setMaximumHeight(fixedHeight);
        m_instrImage->setMinimumWidth(plotWidth);
        m_instrImage->setMaximumWidth(plotWidth);
        return;
    }

    // 计算数据范围
    double minFreq = m_frequencyResponseData.first().first;
    double maxFreq = m_frequencyResponseData.first().first;
    double minAmp = m_frequencyResponseData.first().second;
    double maxAmp = m_frequencyResponseData.first().second;

    for (const auto& point : m_frequencyResponseData) {
        minFreq = qMin(minFreq, point.first);
        maxFreq = qMax(maxFreq, point.first);
        minAmp = qMin(minAmp, point.second);
        maxAmp = qMax(maxAmp, point.second);
    }

    // 频率范围处理
    double freqRange = maxFreq - minFreq;
    if (freqRange < EPSILON) freqRange = MIN_FREQ_RANGE;  // 防止除零

    // 幅度范围固定为指定区间
    minAmp = FIXED_MIN_AMP;
    maxAmp = FIXED_MAX_AMP;
    double ampRange = maxAmp - minAmp;
    if (ampRange < EPSILON) ampRange = MIN_AMP_RANGE;  // 防止除零

    // 创建绘图区域
    int fixedHeight = int(this->height() * 0.4);
    int plotWidth = int(fixedHeight * 2);  // 宽高比约2:1
    
    QPixmap plotPixmap(plotWidth, fixedHeight);
    plotPixmap.fill(Qt::white);
    
    QPainter painter(&plotPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘图区域边距（使用常量）
    int plotAreaWidth = plotWidth - MARGIN_LEFT - MARGIN_RIGHT;
    int plotAreaHeight = fixedHeight - MARGIN_TOP - MARGIN_BOTTOM;

    // 绘制背景网格
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    for (int i = 0; i <= GRID_COUNT_X; ++i) {
        int x = MARGIN_LEFT + i * plotAreaWidth / GRID_COUNT_X;
        painter.drawLine(x, MARGIN_TOP, x, MARGIN_TOP + plotAreaHeight);
    }
    for (int i = 0; i <= GRID_COUNT_Y; ++i) {
        int y = MARGIN_TOP + i * plotAreaHeight / GRID_COUNT_Y;
        painter.drawLine(MARGIN_LEFT, y, MARGIN_LEFT + plotAreaWidth, y);
    }

    // 绘制坐标轴
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(MARGIN_LEFT, MARGIN_TOP + plotAreaHeight, MARGIN_LEFT + plotAreaWidth, MARGIN_TOP + plotAreaHeight);  // X轴
    painter.drawLine(MARGIN_LEFT, MARGIN_TOP, MARGIN_LEFT, MARGIN_TOP + plotAreaHeight);  // Y轴

    // 绘制刻度标签
    painter.setPen(Qt::black);
    QFont labelFont = painter.font();
    labelFont.setPointSize(8);
    painter.setFont(labelFont);

    // X轴标签 (频率)
    for (int i = 0; i <= 5; ++i) {
        int x = MARGIN_LEFT + i * plotAreaWidth / 5;
        double freq = minFreq + i * freqRange / 5;
        QString label;
        if (freq >= 1e9) {
            label = QString::number(freq / 1e9, 'f', 2) + " GHz";
        }
        else if (freq >= 1e6) {
            label = QString::number(freq / 1e6, 'f', 1) + " MHz";
        }
        else if (freq >= 1e3) {
            label = QString::number(freq / 1e3, 'f', 0) + " kHz";
        }
        else {
            label = QString::number(freq, 'f', 0) + " Hz";
        }
        QRect textRect(x - 40, MARGIN_TOP + plotAreaHeight + 5, 80, 20);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }

    // Y轴标签 (幅度)
    for (int i = 0; i <= 4; ++i) {
        int y = MARGIN_TOP + plotAreaHeight - i * plotAreaHeight / 4;
        double amp = minAmp + i * ampRange / 4;
        QString label = QString::number(amp, 'f', 1) + " dBm";
        QRect textRect(5, y - 10, MARGIN_LEFT - 10, 20);
        painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, label);
    }

    // 绘制标题
    QFont titleFont = painter.font();
    titleFont.setPointSize(10);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 5, plotWidth, 20), Qt::AlignCenter, tr("频率响应曲线"));

    // X轴标题
    labelFont.setBold(false);
    painter.setFont(labelFont);
    painter.drawText(QRect(MARGIN_LEFT, fixedHeight - 15, plotAreaWidth, 15), Qt::AlignCenter, tr("频率"));

    // 绘制频响曲线
    if (m_frequencyResponseData.size() > 1) {
        painter.setPen(QPen(QColor(0, 100, 200), 2));
        
        QPolygonF curve;
        for (const auto& point : m_frequencyResponseData) {
            double x = MARGIN_LEFT + (point.first - minFreq) / freqRange * plotAreaWidth;
            double y = MARGIN_TOP + plotAreaHeight - (point.second - minAmp) / ampRange * plotAreaHeight;
            curve << QPointF(x, y);
        }
        
        painter.drawPolyline(curve);
        
        // 绘制数据点标记（如果数据点不太多）
        if (m_frequencyResponseData.size() <= MAX_POINTS_FOR_MARKERS) {
            painter.setPen(QPen(QColor(200, 50, 50), 1));
            painter.setBrush(QColor(200, 50, 50));
            for (const auto& point : m_frequencyResponseData) {
                double x = MARGIN_LEFT + (point.first - minFreq) / freqRange * plotAreaWidth;
                double y = MARGIN_TOP + plotAreaHeight - (point.second - minAmp) / ampRange * plotAreaHeight;
                painter.drawEllipse(QPointF(x, y), 3, 3);
            }
        }
    }

    // 绘制数据点数量信息
    painter.setPen(Qt::darkGray);
    labelFont.setPointSize(8);
    painter.setFont(labelFont);
    QString infoText = tr("数据点: %1").arg(m_frequencyResponseData.size());
    painter.drawText(QRect(plotWidth - 100, 5, 95, 15), Qt::AlignRight, infoText);

    painter.end();

    m_instrImage->setPixmap(plotPixmap);
    m_instrImage->setMinimumHeight(fixedHeight);
    m_instrImage->setMaximumHeight(fixedHeight);
    m_instrImage->setMinimumWidth(plotWidth);
    m_instrImage->setMaximumWidth(plotWidth);
}
