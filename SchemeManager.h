#ifndef SCHEME_MANAGER_H
#define SCHEME_MANAGER_H

#pragma once

#include <QString>
#include <QList>
#include "CommonTypes.h"

class SchemeManager
{
public:
    explicit SchemeManager(const QString& resourceDir = "resource_files/config");
    ~SchemeManager() = default;

    // ============ 基本操作 ============
    /// 保存方案到CSV文件
    bool saveScheme(const TestScheme& scheme);

    /// 从CSV文件加载方案
    bool loadScheme(const QString& schemeName, TestScheme& scheme);

    /// 删除方案
    bool deleteScheme(const QString& schemeName);

    /// 列出所有方案名称
    QStringList listSchemes() const;

    /// 检查方案是否存在
    bool schemeExists(const QString& schemeName) const;

    /// 获取方案文件路径
    QString getSchemeFilePath(const QString& schemeName) const;

private:
    QString m_resourceDir;

    // ============ CSV 序列化 ============
    /// 将 TestScheme 转换为 CSV 字符串
    QString schemeToCSV(const TestScheme& scheme) const;

    /// 从 CSV 字符串转换为 TestScheme
    TestScheme csvToScheme(const QString& csvData) const;

    /// 频率扫描配置序列化为CSV
    QString frequencySweepConfigToCSV(const FrequencySweepConfig& config) const;
    FrequencySweepConfig csvToFrequencySweepConfig(const QString& csvLine) const;

    /// 频谱分析仪配置序列化为CSV
    QString spectrumAnalyzerConfigToCSV(const SpectrumAnalyzerConfig& config) const;
    SpectrumAnalyzerConfig csvToSpectrumAnalyzerConfig(const QString& csvLine) const;

    /// 信号发生器配置序列化为CSV
    QString signalGeneratorConfigToCSV(const SignalGeneratorConfig& config) const;
    SignalGeneratorConfig csvToSignalGeneratorConfig(const QString& csvLine) const;

    // ============ 辅助函数 ============
    /// 确保资源目录存在
    bool ensureResourceDirExists();

    /// 获取安全的文件名
    QString sanitizeFileName(const QString& schemeName) const;

    /// CSV 转义（处理逗号和引号）
    QString escapeCSV(const QString& field) const;
    QString unescapeCSV(const QString& field) const;
};

#endif // SCHEME_MANAGER_H