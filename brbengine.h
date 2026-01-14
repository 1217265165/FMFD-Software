#pragma once
#include <QObject>
#include <QVector>
#include <QMap>
#include <QStringList>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariant>
#include "CommonTypes.h"

struct BRBRule {
    QVector<double> centers;
    QVector<double> sigmas;
    QMap<QString, double> belief;
    double weight = 1.0;
};

class BRBEngine : public QObject
{
    Q_OBJECT
public:
    explicit BRBEngine(QObject* parent = nullptr);

    QStringList moduleNames() const;
    void addRule(const BRBRule& r, const QStringList& attributeOrder);
    void infer(const QMap<QString, double>& features);

    // ============ JSON解析相关 ============
    // 安全类型转换函数（JSON值可能是number或string）
    static double toDoubleSafe(const QJsonValue& val, double defaultVal = 0.0);
    static bool toBoolSafe(const QJsonValue& val, bool defaultVal = false);
    static QString toStringSafe(const QJsonValue& val, const QString& defaultVal = QString());
    static int toIntSafe(const QJsonValue& val, int defaultVal = 0);

    // 解析 object<string, double> 类型的 JSON 对象
    static QMap<QString, double> parseDoubleMap(const QJsonObject& obj);

    // 从 JSON 文件加载诊断结果
    static DiagnosisResult loadDiagnosisResult(const QString& jsonPath, QString* errorMsg = nullptr);

    // 从 labels.json 加载 Ground Truth
    static bool loadGroundTruth(const QString& labelsJsonPath, const QString& sampleId,
                                QString& gtSystemFault, QString& gtModule);

signals:
    void diagnosisReady(const QMap<QString, double>& moduleProb);
    void logMessage(const QString& msg);

private:
    double gaussianMatch(double x, double center, double sigma) const;

    QVector<BRBRule> m_rules;
    QStringList m_attributesOrder;
    QStringList m_modules;
};