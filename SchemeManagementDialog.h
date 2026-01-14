#pragma once

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <memory>
#include "CommonTypes.h"

// ✅ 添加这个前向声明
class SchemeManager;


class SchemeManagementDialog : public QDialog {
    Q_OBJECT

public:
    explicit SchemeManagementDialog(QWidget* parent = nullptr);

    // 获取选中的方案
    TestScheme getSelectedScheme() const;

    // 设置当前方案
    void setCurrentScheme(const TestScheme& scheme);

signals:
    // ✅ 新增：请求编辑配置的信号（使用值传递以确保信号槽正常工作）
    void requestEditFrequencySweep();
    void requestEditSpectrumAnalyzer();
    void requestEditSignalGenerator();

private slots:
    void onSaveScheme();
    void onLoadScheme();
    void onDeleteScheme();
    /*void onListItemSelected();*/
    void onSchemeListContextMenu(const QPoint& pos);  // ✅ 右键菜单
    void onOpenSchemeFile();                          // ✅ 打开文件
    
    // ✅ 新增：配置编辑槽函数
    void onEditFrequencySweep();
    void onEditSpectrumAnalyzer();
    void onEditSignalGenerator();

private:
    
    void setupUI();
    void refreshSchemeList();
    void setupContextMenu();  // ✅ 初始化菜单

    std::unique_ptr<SchemeManager> m_schemeManager;


    QListWidget* m_schemeList;
    QLineEdit* m_schemeNameEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_loadBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
    
    // ✅ 新增：配置编辑按钮
    QPushButton* m_editFreqBtn;
    QPushButton* m_editSaBtn;
    QPushButton* m_editSgBtn;

    TestScheme m_currentScheme;
    QString m_schemesDir;
};
