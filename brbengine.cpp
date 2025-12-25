#include "brbengine.h"
#include <QtMath>

BRBEngine::BRBEngine(QObject* parent)
    : QObject(parent)
{
    // 使用你提供的更完整模块列表
    m_modules = QStringList({
        // RF 板子模块（细分）
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
        // 数字IF 板
        "IF_Amplifier", "ADC", "FPGA_DSP",
        // 电源
        "Power_Module"
        });

    // attributes order unchanged (你可以按需要调整)
    m_attributesOrder = QStringList({ "AmplitudeErr", "FrequencyErr", "PhaseNoise", "RefLevelOffset" });

    // 示例规则：这里保留少量示例规则，真实项目请按经验/训练数据修改
    BRBRule r1;
    r1.centers = { 2.0, 0.0, 1.0, 0.5 };
    r1.sigmas = { 1.5, 50.0, 2.0, 1.0 };
    // belief map 调整为对新模块的分布（示例）
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