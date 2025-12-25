#pragma once

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>

class UsageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UsageDialog(QWidget* parent = nullptr);
    ~UsageDialog();

private:
    void initializeUI();
    QString getUsageText();

    QTextEdit* m_textEdit;
    QPushButton* m_closeButton;
};