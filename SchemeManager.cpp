#include "SchemeManager.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringList>

SchemeManager::SchemeManager(const QString& resourceDir)
    : m_resourceDir(resourceDir)
{
    ensureResourceDirExists();
}

bool SchemeManager::saveScheme(const TestScheme& scheme)
{
    if (!ensureResourceDirExists()) {
        qWarning() << "Failed to create resource directory";
        return false;
    }

    QString csvData = schemeToCSV(scheme);
    QString filePath = getSchemeFilePath(scheme.name);
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QTextStream out(&file);
    out << csvData;
    file.close();

    qDebug() << "Scheme saved successfully:" << filePath;
    return true;
}

bool SchemeManager::loadScheme(const QString& schemeName, TestScheme& scheme)
{
    QString filePath = getSchemeFilePath(schemeName);
    QFile file(filePath);

    if (!file.exists()) {
        qWarning() << "Scheme file does not exist:" << filePath;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        return false;
    }

    QTextStream in(&file);

    QString csvData = in.readAll();
    file.close();

    if (csvData.isEmpty()) {
        qWarning() << "CSV file is empty:" << filePath;
        return false;
    }

    scheme = csvToScheme(csvData);
    qDebug() << "Scheme loaded successfully:" << filePath;
    return true;
}

bool SchemeManager::deleteScheme(const QString& schemeName)
{
    QString filePath = getSchemeFilePath(schemeName);
    QFile file(filePath);

    if (!file.exists()) {
        qWarning() << "Scheme file does not exist:" << filePath;
        return false;
    }

    if (!file.remove()) {
        qWarning() << "Failed to delete scheme file:" << filePath;
        return false;
    }

    qDebug() << "Scheme deleted successfully:" << filePath;
    return true;
}

QStringList SchemeManager::listSchemes() const
{
    QStringList schemes;
    QDir dir(m_resourceDir);

    if (!dir.exists()) {
        return schemes;
    }

    QStringList filters;
    filters << "*.csv";
    dir.setNameFilters(filters);

    QStringList files = dir.entryList(QDir::Files);

    for (const QString& file : files) {
        // 移除 .csv 扩展名得到方案名称
        schemes.append(file.left(file.length() - 4));
    }

    return schemes;
}

bool SchemeManager::schemeExists(const QString& schemeName) const
{
    QString filePath = getSchemeFilePath(schemeName);
    return QFile::exists(filePath);
}

QString SchemeManager::getSchemeFilePath(const QString& schemeName) const
{
    QString safeName = sanitizeFileName(schemeName);
    return m_resourceDir + "/" + safeName + ".csv";
}

QString SchemeManager::schemeToCSV(const TestScheme& scheme) const
{
    QString csv;
    QTextStream stream(&csv);

    // 第一行：方案名称
    stream << "SCHEME_NAME," << escapeCSV(scheme.name) << "\n";

    // 第二行：频率扫描配置
    stream << "FREQUENCY_SWEEP_CONFIG\n";
    stream << frequencySweepConfigToCSV(scheme.sweepConfig);

    // 频谱分析仪配置
    stream << "SPECTRUM_ANALYZER_CONFIG\n";
    stream << spectrumAnalyzerConfigToCSV(scheme.saConfig);

    // 信号发生器配置
    stream << "SIGNAL_GENERATOR_CONFIG\n";
    stream << signalGeneratorConfigToCSV(scheme.sgConfig);

    return csv;
}

TestScheme SchemeManager::csvToScheme(const QString& csvData) const
{
    TestScheme scheme;
    QStringList lines = csvData.split('\n', Qt::SkipEmptyParts);

    int i = 0;
    while (i < lines.size()) {
        QString line = lines[i].trimmed();

        if (line.startsWith("SCHEME_NAME")) {
            QStringList parts = line.split(',');
            if (parts.size() > 1) {
                scheme.name = unescapeCSV(parts[1]);
            }
            i++;
        }
        else if (line.startsWith("FREQUENCY_SWEEP_CONFIG")) {
            i++;
            if (i < lines.size()) {
                scheme.sweepConfig = csvToFrequencySweepConfig(lines[i]);
                i++;
            }
        }
        else if (line.startsWith("SPECTRUM_ANALYZER_CONFIG")) {
            i++;
            if (i < lines.size()) {
                scheme.saConfig = csvToSpectrumAnalyzerConfig(lines[i]);
                i++;
            }
        }
        else if (line.startsWith("SIGNAL_GENERATOR_CONFIG")) {
            i++;
            if (i < lines.size()) {
                scheme.sgConfig = csvToSignalGeneratorConfig(lines[i]);
                i++;
            }
        }
        else {
            i++;
        }
    }

    return scheme;
}

QString SchemeManager::frequencySweepConfigToCSV(const FrequencySweepConfig& config) const
{
    QString csv;
    QTextStream stream(&csv);

    stream << "mode," << static_cast<int>(config.mode) << "\n";
    stream << "freqFilePath," << escapeCSV(QString::fromStdString(config.freqFilePath)) << "\n";
    stream << "segments_count," << config.segments.size() << "\n";

    for (size_t i = 0; i < config.segments.size(); ++i) {
        const auto& seg = config.segments[i];
        stream << "segment_" << i << "_startHz," << seg.startHz << "\n";
        stream << "segment_" << i << "_stopHz," << seg.stopHz << "\n";
        stream << "segment_" << i << "_stepHz," << seg.stepHz << "\n";
    }

    return csv;
}

FrequencySweepConfig SchemeManager::csvToFrequencySweepConfig(const QString& csvLine) const
{
    FrequencySweepConfig config;
    QStringList lines = csvLine.split('\n', Qt::SkipEmptyParts);

    size_t segmentCount = 0;

    for (const QString& line : lines) {
        QStringList parts = line.split(',');
        if (parts.isEmpty()) continue;

        QString key = parts[0].trimmed();
        QString value = parts.size() > 1 ? parts[1].trimmed() : "";

        if (key == "mode") {
            config.mode = static_cast<FrequencySweepConfig::Mode>(value.toInt());
        }
        else if (key == "freqFilePath") {
            config.freqFilePath = unescapeCSV(value).toStdString();
        }
        else if (key == "segments_count") {
            segmentCount = value.toULongLong();
        }
    }

    config.segments.clear();
    for (size_t i = 0; i < segmentCount; ++i) {
        FrequencySweepSegment seg;
        bool found = false;

        for (const QString& line : lines) {
            QStringList parts = line.split(',');
            if (parts.size() < 2) continue;

            QString key = parts[0].trimmed();
            QString value = parts[1].trimmed();

            if (key == QString("segment_%1_startHz").arg(i)) {
                seg.startHz = value.toDouble();
                found = true;
            }
            else if (key == QString("segment_%1_stopHz").arg(i)) {
                seg.stopHz = value.toDouble();
            }
            else if (key == QString("segment_%1_stepHz").arg(i)) {
                seg.stepHz = value.toDouble();
            }
        }

        if (found) {
            config.segments.push_back(seg);
        }
    }

    return config;
}

QString SchemeManager::spectrumAnalyzerConfigToCSV(const SpectrumAnalyzerConfig& config) const
{
    QString csv;
    QTextStream stream(&csv);

    stream << "refLevelDbm," << config.refLevelDbm << "\n";
    stream << "attenuatorDb," << config.attenuatorDb << "\n";
    stream << "preampEnabled," << (config.preampEnabled ? "1" : "0") << "\n";
    stream << "rbwAutoMode," << (config.rbwAutoMode ? "1" : "0") << "\n";
    stream << "rbwHz," << config.rbwHz << "\n";
    stream << "vbwSameAsRbw," << (config.vbwSameAsRbw ? "1" : "0") << "\n";
    stream << "vbwHz," << config.vbwHz << "\n";
    stream << "couplingMode," << escapeCSV(config.couplingMode) << "\n";
    stream << "measurementMode," << escapeCSV(config.measurementMode) << "\n";
    stream << "amplitudeDbm," << config.amplitudeDbm << "\n";
    stream << "resolutionHz," << config.resolutionHz << "\n";

    return csv;
}

SpectrumAnalyzerConfig SchemeManager::csvToSpectrumAnalyzerConfig(const QString& csvLine) const
{
    SpectrumAnalyzerConfig config;
    QStringList lines = csvLine.split('\n', Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        QStringList parts = line.split(',');
        if (parts.size() < 2) continue;

        QString key = parts[0].trimmed();
        QString value = parts[1].trimmed();

        if (key == "refLevelDbm") config.refLevelDbm = value.toDouble();
        else if (key == "attenuatorDb") config.attenuatorDb = value.toInt();
        else if (key == "preampEnabled") config.preampEnabled = (value == "1");
        else if (key == "rbwAutoMode") config.rbwAutoMode = (value == "1");
        else if (key == "rbwHz") config.rbwHz = value.toDouble();
        else if (key == "vbwSameAsRbw") config.vbwSameAsRbw = (value == "1");
        else if (key == "vbwHz") config.vbwHz = value.toDouble();
        else if (key == "couplingMode") config.couplingMode = unescapeCSV(value);
        else if (key == "measurementMode") config.measurementMode = unescapeCSV(value);
        else if (key == "amplitudeDbm") config.amplitudeDbm = value.toDouble();
        else if (key == "resolutionHz") config.resolutionHz = value.toDouble();
    }

    return config;
}

QString SchemeManager::signalGeneratorConfigToCSV(const SignalGeneratorConfig& config) const
{
    QString csv;
    QTextStream stream(&csv);

    stream << "powerDbm," << config.powerDbm << "\n";
    stream << "frequencyCompensationEnabled," << (config.frequencyCompensationEnabled ? "1" : "0") << "\n";
    stream << "powerCompensationEnabled," << (config.powerCompensationEnabled ? "1" : "0") << "\n";
    stream << "outputConnector," << escapeCSV(config.outputConnector) << "\n";
    stream << "powerFlatteningEnabled," << (config.powerFlatteningEnabled ? "1" : "0") << "\n";
    stream << "modulationMode," << escapeCSV(config.modulationMode) << "\n";
    stream << "outputEnabled," << (config.outputEnabled ? "1" : "0") << "\n";

    return csv;
}

SignalGeneratorConfig SchemeManager::csvToSignalGeneratorConfig(const QString& csvLine) const
{
    SignalGeneratorConfig config;
    QStringList lines = csvLine.split('\n', Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        QStringList parts = line.split(',');
        if (parts.size() < 2) continue;

        QString key = parts[0].trimmed();
        QString value = parts[1].trimmed();

        if (key == "powerDbm") config.powerDbm = value.toDouble();
        else if (key == "frequencyCompensationEnabled") config.frequencyCompensationEnabled = (value == "1");
        else if (key == "powerCompensationEnabled") config.powerCompensationEnabled = (value == "1");
        else if (key == "outputConnector") config.outputConnector = unescapeCSV(value);
        else if (key == "powerFlatteningEnabled") config.powerFlatteningEnabled = (value == "1");
        else if (key == "modulationMode") config.modulationMode = unescapeCSV(value);
        else if (key == "outputEnabled") config.outputEnabled = (value == "1");
    }

    return config;
}

bool SchemeManager::ensureResourceDirExists()
{
    QDir dir;
    if (!dir.exists(m_resourceDir)) {
        if (!dir.mkpath(m_resourceDir)) {
            qWarning() << "Failed to create resource directory:" << m_resourceDir;
            return false;
        }
        qDebug() << "Resource directory created:" << m_resourceDir;
    }
    return true;
}

QString SchemeManager::sanitizeFileName(const QString& schemeName) const
{
    QString safe = schemeName;
    safe.replace("/", "_");
    safe.replace("\\", "_");
    safe.replace(":", "_");
    safe.replace("*", "_");
    safe.replace("?", "_");
    safe.replace("\"", "_");
    safe.replace("<", "_");
    safe.replace(">", "_");
    safe.replace("|", "_");
    return safe;
}

QString SchemeManager::escapeCSV(const QString& field) const
{
    // 如果字段包含逗号、引号或换行符，用引号包围，并将引号转义
    if (field.contains(',') || field.contains('"') || field.contains('\n')) {
        QString escaped = field;
        escaped.replace("\"", "\"\"");
        return "\"" + escaped + "\"";
    }
    return field;
}

QString SchemeManager::unescapeCSV(const QString& field) const
{
    QString result = field;
    // 移除外层引号
    if (result.startsWith("\"") && result.endsWith("\"")) {
        result = result.mid(1, result.length() - 2);
    }
    // 反转义引号
    result.replace("\"\"", "\"");
    return result;
}