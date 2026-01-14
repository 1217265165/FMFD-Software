#include "brbengine.h"
#include <QtMath>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

BRBEngine::BRBEngine(QObject* parent)
    : QObject(parent)
{
    // 使用提供的高级模块列表
    m_modules = QStringList({
        // RF 链路模块（细分）
        "Attenuator", "Preamp", "Lowband_LPF", "Lowband_Mixer1", "Lowband_Filter1", "Lowband_Mixer2", "Lowband_Filter2",
        "Highband_YTF", "Highband_Mixer",
        // 时钟系统
        "Clock_Oscillator", "Clock_Synth",
        // 本振系统
        "LO_Source", "LO_Mixer",
        // 校准系统
        "Cal_Source", "Cal_Memory", "Cal_Switch",
        // 控制与交互
        "CPU_Control", "Ext_Interface", "Display",
        // 中频IF 段
        "IF_Amplifier", "ADC", "FPGA_DSP",
        // 电源
        "Power_Module"
        });

    // attributes order unchanged (后续可按需添加)
    m_attributesOrder = QStringList({ "AmplitudeErr", "FrequencyErr", "PhaseNoise", "RefLevelOffset" });

    // 示例规则：这里保留示例规则，具体实验可按数据/训练结果修改
    BRBRule r1;
    r1.centers = { 2.0, 0.0, 1.0, 0.5 };
    r1.sigmas = { 1.5, 50.0, 2.0, 1.0 };
    // belief map 设置为各模块的分布示例：
    r1.belief = {
        {"Attenuator", 0.6}, {"Preamp", 0.15}, {"Lowband_Filter1", 0.05},
        {"Highband_YTF", 0.05}, {"ADC", 0.05}, {"Power_Module", 0.05}
    };
    r1.weight = 1.0;
    addRule(r1, m_attributesOrder);

    BRBRule r2;
    r2.centers = { -1.8, 0.0, 0.5, -0.3 };
    r2.sigmas = { 1.0, 50.0, 1.5, 0.8 };
    r2.belief = {
        {"Preamp", 0.5}, {"Attenuator", 0.2}, {"Lowband_Mixer1", 0.1},
        {"IF_Amplifier", 0.1}, {"ADC", 0.05}, {"Power_Module", 0.05}
    };
    r2.weight = 0.9;
    addRule(r2, m_attributesOrder);

    BRBRule r3;
    r3.centers = { 0.0, 30.0, 2.0, 0.0 };
    r3.sigmas = { 2.0, 40.0, 2.5, 1.5 };
    r3.belief = {
        {"Lowband_Filter2", 0.4}, {"Highband_Mixer", 0.2}, {"Highband_YTF", 0.1},
        {"FPGA_DSP", 0.1}, {"ADC", 0.1}, {"Power_Module", 0.1}
    };
    r3.weight = 0.8;
    addRule(r3, m_attributesOrder);
}

void BRBEngine::addRule(const BRBRule& r, const QStringList& attributeOrder)
{
    Q_UNUSED(attributeOrder);
    m_rules.append(r);
    emit logMessage(QString("BRB: added rule (weight=%1)").arg(r.weight));
}

QStringList BRBEngine::moduleNames() const { return m_modules; }

double BRBEngine::gaussianMatch(double x, double center, double sigma) const
{
    if (sigma <= 0.0) return (x == center) ? 1.0 : 0.0;
    double diff = x - center;
    double v = qExp(-0.5 * (diff * diff) / (sigma * sigma));
    return v;
}

void BRBEngine::infer(const QMap<QString, double>& features)
{
    QMap<QString, double> aggregated;
    double totalActivation = 0.0;
    for (const QString& m : m_modules) aggregated[m] = 0.0;

    for (const BRBRule& r : m_rules) {
        double activation = 1.0;
        for (int i = 0; i < r.centers.size() && i < m_attributesOrder.size(); ++i) {
            QString attr = m_attributesOrder[i];
            double val = features.contains(attr) ? features[attr] : 0.0;
            double match = gaussianMatch(val, r.centers[i], r.sigmas[i]);
            activation *= match;
        }
        double weightedAct = activation * r.weight;
        if (weightedAct <= 1e-9) continue;
        totalActivation += weightedAct;
        for (auto it = r.belief.constBegin(); it != r.belief.constEnd(); ++it) {
            aggregated[it.key()] += weightedAct * it.value();
        }
    }

    QMap<QString, double> finalProb;
    if (totalActivation <= 1e-9) {
        double uniform = 1.0 / double(m_modules.size());
        for (const QString& m : m_modules) finalProb[m] = uniform * 0.05;
        emit logMessage(QStringLiteral("BRB: no strong rule activation; low-confidence uniform result."));
    }
    else {
        double sum = 0.0;
        for (const QString& m : m_modules) {
            double v = aggregated.value(m, 0.0) / totalActivation;
            finalProb[m] = v;
            sum += v;
        }
        if (sum > 1e-9) {
            for (const QString& m : m_modules) finalProb[m] /= sum;
        }
        else {
            double uniform = 1.0 / double(m_modules.size());
            for (const QString& m : m_modules) finalProb[m] = uniform;
        }
    }

    emit diagnosisReady(finalProb);
}

// ============ JSON解析静态方法实现 ============

double BRBEngine::toDoubleSafe(const QJsonValue& val, double defaultVal)
{
    if (val.isDouble()) {
        return val.toDouble();
    }
    else if (val.isString()) {
        bool ok = false;
        double d = val.toString().toDouble(&ok);
        return ok ? d : defaultVal;
    }
    return defaultVal;
}

bool BRBEngine::toBoolSafe(const QJsonValue& val, bool defaultVal)
{
    if (val.isBool()) {
        return val.toBool();
    }
    else if (val.isString()) {
        QString s = val.toString().toLower();
        if (s == "true" || s == "1" || s == "yes") return true;
        if (s == "false" || s == "0" || s == "no") return false;
        return defaultVal;
    }
    else if (val.isDouble()) {
        return val.toDouble() != 0.0;
    }
    return defaultVal;
}

QString BRBEngine::toStringSafe(const QJsonValue& val, const QString& defaultVal)
{
    if (val.isString()) {
        return val.toString();
    }
    else if (val.isDouble()) {
        return QString::number(val.toDouble());
    }
    else if (val.isBool()) {
        return val.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }
    return defaultVal;
}

int BRBEngine::toIntSafe(const QJsonValue& val, int defaultVal)
{
    if (val.isDouble()) {
        return static_cast<int>(val.toDouble());
    }
    else if (val.isString()) {
        bool ok = false;
        int i = val.toString().toInt(&ok);
        return ok ? i : defaultVal;
    }
    return defaultVal;
}

QMap<QString, double> BRBEngine::parseDoubleMap(const QJsonObject& obj)
{
    QMap<QString, double> result;
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        result[it.key()] = toDoubleSafe(it.value(), 0.0);
    }
    return result;
}

DiagnosisResult BRBEngine::loadDiagnosisResult(const QString& jsonPath, QString* errorMsg)
{
    DiagnosisResult result;

    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMsg) *errorMsg = QStringLiteral("无法打开文件: ") + jsonPath;
        return result;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (errorMsg) *errorMsg = QStringLiteral("JSON解析错误: ") + parseError.errorString();
        return result;
    }

    if (!doc.isObject()) {
        if (errorMsg) *errorMsg = QStringLiteral("JSON根节点不是对象");
        return result;
    }

    QJsonObject root = doc.object();

    // 解析 input_file
    result.inputFile = toStringSafe(root.value("input_file"));

    // 解析 data_points
    result.dataPoints = toIntSafe(root.value("data_points"), 0);

    // 解析 frequency_range
    if (root.contains("frequency_range") && root["frequency_range"].isObject()) {
        QJsonObject freqRange = root["frequency_range"].toObject();
        result.frequencyRange.min = toDoubleSafe(freqRange.value("min"), 0.0);
        result.frequencyRange.max = toDoubleSafe(freqRange.value("max"), 0.0);
    }

    // 解析 features
    if (root.contains("features") && root["features"].isObject()) {
        result.features = parseDoubleMap(root["features"].toObject());
    }

    // 解析 system_diagnosis
    if (root.contains("system_diagnosis") && root["system_diagnosis"].isObject()) {
        QJsonObject sysDiag = root["system_diagnosis"].toObject();

        // probabilities
        if (sysDiag.contains("probabilities") && sysDiag["probabilities"].isObject()) {
            result.systemDiagnosis.probabilities = parseDoubleMap(sysDiag["probabilities"].toObject());
        }

        // predicted_class - 字符串类型
        result.systemDiagnosis.predictedClass = toStringSafe(sysDiag.value("predicted_class"));

        // max_prob - double 0~1
        result.systemDiagnosis.maxProb = toDoubleSafe(sysDiag.value("max_prob"), 0.0);

        // is_normal - bool
        result.systemDiagnosis.isNormal = toBoolSafe(sysDiag.value("is_normal"), false);
    }

    // 解析 module_diagnosis（嵌套结构：包含 probabilities, topk, disabled_modules）
    if (root.contains("module_diagnosis") && root["module_diagnosis"].isObject()) {
        QJsonObject modDiagObj = root["module_diagnosis"].toObject();
        
        // 优先从 probabilities 子对象解析
        if (modDiagObj.contains("probabilities") && modDiagObj["probabilities"].isObject()) {
            result.moduleDiagnosis = parseDoubleMap(modDiagObj["probabilities"].toObject());
        } else {
            // 兼容旧格式：直接是 object<string, double>
            result.moduleDiagnosis = parseDoubleMap(modDiagObj);
        }
    }

    // 解析 evidence (可选字段)
    if (root.contains("evidence") && root["evidence"].isObject()) {
        QJsonObject evidenceObj = root["evidence"].toObject();
        for (auto it = evidenceObj.begin(); it != evidenceObj.end(); ++it) {
            result.evidence[it.key()] = it.value().toVariant();
        }
    }

    return result;
}

bool BRBEngine::loadGroundTruth(const QString& labelsJsonPath, const QString& sampleId,
                                 QString& gtSystemFault, QString& gtModule)
{
    QFile file(labelsJsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains(sampleId)) {
        return false;
    }

    QJsonValue sampleVal = root.value(sampleId);
    if (!sampleVal.isObject()) {
        return false;
    }

    QJsonObject sampleObj = sampleVal.toObject();
    gtSystemFault = toStringSafe(sampleObj.value("system_fault_class"));
    gtModule = toStringSafe(sampleObj.value("module"));

    return true;
}