#pragma once
#include <QObject>
#include <QVector>
#include <QMap>
#include <QStringList>

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

signals:
    void diagnosisReady(const QMap<QString, double>& moduleProb);
    void logMessage(const QString& msg);

private:
    double gaussianMatch(double x, double center, double sigma) const;

    QVector<BRBRule> m_rules;
    QStringList m_attributesOrder;
    QStringList m_modules;
};