#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QGroupBox>
#include "SchemeManagementDialog.h"
#include "SchemeManager.h"
#include "CommonTypes.h"
#include "FMFD.h"
#include <QDir>


SchemeManagementDialog::SchemeManagementDialog(QWidget* parent)
    : QDialog(parent)
    , m_schemeManager(std::make_unique<SchemeManager>("resource_files/config"))
{
    setupUI();
    setupContextMenu();  // ✅ 新增：设置右键菜单
}

void SchemeManagementDialog::setupUI()
{
    setWindowTitle("方案管理");
    setGeometry(100, 100, 700, 550);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // ✅ 配置编辑区域移到最上面
    QGroupBox* configGroup = new QGroupBox("当前方案配置");
    QHBoxLayout* configLayout = new QHBoxLayout();
    
    m_editFreqBtn = new QPushButton("频率扫描");
    m_editSaBtn = new QPushButton("频谱分析仪");
    m_editSgBtn = new QPushButton("信号发生器");
    
    configLayout->addWidget(m_editFreqBtn);
    configLayout->addWidget(m_editSaBtn);
    configLayout->addWidget(m_editSgBtn);
    configGroup->setLayout(configLayout);
    mainLayout->addWidget(configGroup);

    // 方案名称编辑
    mainLayout->addWidget(new QLabel("方案名称："));
    m_schemeNameEdit = new QLineEdit();
    mainLayout->addWidget(m_schemeNameEdit);

    // 方案列表
    mainLayout->addWidget(new QLabel("已保存方案（右键打开文件）："));
    m_schemeList = new QListWidget();
    m_schemeList->setContextMenuPolicy(Qt::CustomContextMenu);  // ✅ 启用自定义右键菜单
    mainLayout->addWidget(m_schemeList);

    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton("保存");
    m_loadBtn = new QPushButton("加载");
    m_deleteBtn = new QPushButton("删除");
    
    buttonLayout->addWidget(m_saveBtn);
    buttonLayout->addWidget(m_loadBtn);
    buttonLayout->addWidget(m_deleteBtn);
    buttonLayout->addStretch();
    
    m_okBtn = new QPushButton("确定");
    m_cancelBtn = new QPushButton("取消");
    buttonLayout->addWidget(m_okBtn);
    buttonLayout->addWidget(m_cancelBtn);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    // 连接信号
    connect(m_saveBtn, &QPushButton::clicked, this, &SchemeManagementDialog::onSaveScheme);
    connect(m_loadBtn, &QPushButton::clicked, this, &SchemeManagementDialog::onLoadScheme);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SchemeManagementDialog::onDeleteScheme);
    connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    
    // ✅ 配置编辑按钮信号
    connect(m_editFreqBtn, &QPushButton::clicked, this, &SchemeManagementDialog::onEditFrequencySweep);
    connect(m_editSaBtn, &QPushButton::clicked, this, &SchemeManagementDialog::onEditSpectrumAnalyzer);
    connect(m_editSgBtn, &QPushButton::clicked, this, &SchemeManagementDialog::onEditSignalGenerator);

    refreshSchemeList();
}

// ✅ 新增：设置右键菜单
void SchemeManagementDialog::setupContextMenu()
{
    // 连接右键菜单信号
    connect(m_schemeList, &QListWidget::customContextMenuRequested,
            this, &SchemeManagementDialog::onSchemeListContextMenu);
}

// ✅ 新增：右键菜单处理
void SchemeManagementDialog::onSchemeListContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = m_schemeList->itemAt(pos);
    if (!item) return;

    // 创建右键菜单
    QMenu contextMenu;
    QAction* openAction = contextMenu.addAction("打开文件");
    QAction* deleteAction = contextMenu.addAction("删除方案");

    // 执行菜单
    QAction* selectedAction = contextMenu.exec(m_schemeList->mapToGlobal(pos));

    if (selectedAction == openAction) {
        m_schemeList->setCurrentItem(item);
        onOpenSchemeFile();  // ✅ 打开文件
    } else if (selectedAction == deleteAction) {
        m_schemeList->setCurrentItem(item);
        onDeleteScheme();
    }
}

// ✅ 新增：打开方案文件
void SchemeManagementDialog::onOpenSchemeFile()
{
    QListWidgetItem* currentItem = m_schemeList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "提示", "请先选择一个方案");
        return;
    }

    QString schemeName = currentItem->text();
    
    // ✅ 方案 CSV 文件路径（保存在 resource_files/config 目录）
    QString schemeFilePath = QString("%1/resource_files/config/%2.csv")
        .arg(QDir::homePath())
        .arg(schemeName);

    // 如果文件不存在，尝试其他可能的路径
    if (!QFile::exists(schemeFilePath)) {
        // 尝试当前项目目录
        schemeFilePath = QString("./resource_files/config/%1.csv").arg(schemeName);
    }

    if (!QFile::exists(schemeFilePath)) {
        QMessageBox::critical(this, "错误", 
            QString("找不到方案文件：%1").arg(schemeFilePath));
        return;
    }

    // ✅ 用默认应用打开文件（Excel/记事本等）
    bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(schemeFilePath));
    
    if (!success) {
        QMessageBox::warning(this, "错误", "无法打开文件，请检查文件关联");
    }
}

void SchemeManagementDialog::refreshSchemeList()
{
    m_schemeList->clear();
    
    if (!m_schemeManager) return;
    
    QStringList schemes = m_schemeManager->listSchemes();
    for (const QString& schemeName : schemes) {
        m_schemeList->addItem(schemeName);
    }
}

// ✅ 其他槽函数的实现
void SchemeManagementDialog::onSaveScheme()
{
    QString schemeName = m_schemeNameEdit->text().trimmed();
    
    if (schemeName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入方案名称");
        return;
    }
    
    if (!m_schemeManager) return;
    
    // 检查是否覆盖现有方案
    if (m_schemeManager->schemeExists(schemeName)) {
        if (QMessageBox::question(this, "确认", 
            QString("方案 \"%1\" 已存在，是否覆盖？").arg(schemeName),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return;
        }
    }
    
    // 设置方案名称（同时设置 name 和 schemeName 以保持兼容性）
    m_currentScheme.name = schemeName;
    m_currentScheme.schemeName = schemeName;
    
    // 保存方案
    if (m_schemeManager->saveScheme(m_currentScheme)) {
        QMessageBox::information(this, "成功", 
            QString("方案 \"%1\" 已成功保存到:\nresource_files/config/%2.csv").arg(schemeName).arg(schemeName));
        refreshSchemeList();
        
        // 在列表中选中刚保存的方案
        QList<QListWidgetItem*> items = m_schemeList->findItems(schemeName, Qt::MatchExactly);
        if (!items.isEmpty()) {
            m_schemeList->setCurrentItem(items.first());
        }
    } else {
        QMessageBox::critical(this, "错误", 
            QString("保存方案失败！\n请检查 resource_files/config 目录是否可写。"));
    }
}

void SchemeManagementDialog::onLoadScheme()
{
    QListWidgetItem* currentItem = m_schemeList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "提示", "请先选择要加载的方案");
        return;
    }
    
    QString schemeName = currentItem->text();
    TestScheme scheme;
    
    if (!m_schemeManager) return;
    
    if (m_schemeManager->loadScheme(schemeName, scheme)) {
        m_currentScheme = scheme;
        // 使用 name 字段（优先）或 schemeName 字段
        QString displayName = !scheme.name.isEmpty() ? scheme.name : scheme.schemeName;
        m_schemeNameEdit->setText(displayName);
        QMessageBox::information(this, "成功", QString("方案 \"%1\" 已加载").arg(displayName));
    } else {
        QMessageBox::critical(this, "错误", "加载方案失败");
    }
}

void SchemeManagementDialog::onDeleteScheme()
{
    QListWidgetItem* currentItem = m_schemeList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "提示", "请先选择要删除的方案");
        return;
    }
    
    QString schemeName = currentItem->text();
    
    int ret = QMessageBox::question(this, "确认删除", 
        QString("确定要删除方案 '%1' 吗？").arg(schemeName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (!m_schemeManager) return;
        
        if (m_schemeManager->deleteScheme(schemeName)) {
            QMessageBox::information(this, "成功", "方案已删除");
            refreshSchemeList();
        } else {
            QMessageBox::critical(this, "错误", "删除方案失败");
        }
    }
}

TestScheme SchemeManagementDialog::getSelectedScheme() const
{
    return m_currentScheme;
}

void SchemeManagementDialog::setCurrentScheme(const TestScheme& scheme)
{
    m_currentScheme = scheme;
    // 使用 name 字段（优先）或 schemeName 字段
    QString displayName = !scheme.name.isEmpty() ? scheme.name : scheme.schemeName;
    m_schemeNameEdit->setText(displayName);
}

// ✅ 新增：配置编辑槽函数实现
void SchemeManagementDialog::onEditFrequencySweep()
{
    emit requestEditFrequencySweep();
}

void SchemeManagementDialog::onEditSpectrumAnalyzer()
{
    emit requestEditSpectrumAnalyzer();
}

void SchemeManagementDialog::onEditSignalGenerator()
{
    emit requestEditSignalGenerator();
}