
#pragma once

#include <QString>
#include <vector>
#include <chrono>

// ============ 频率扫描相关 ============
struct FrequencySweepSegment {
    double startHz = 10e3;
    double stopHz = 1e6;
    double stepHz = 10e3;
};

struct FrequencySweepConfig {
    enum Mode {
        FileImport = 0,
        SegmentGeneration = 1
    };

    Mode mode = SegmentGeneration;
    std::string freqFilePath;
    std::vector<FrequencySweepSegment> segments = {
        {10e3, 1e6, 10e3},
        {1e6, 10e6, 1e6},
        {10e6, 500e6, 1e7},
        {500e6, 6.7e10, 50e6}
    };
};

// ============ 频谱分析仪相关 ============
struct SpectrumAnalyzerConfig {
    double refLevelDbm = 0.0;
    int attenuatorDb = 0;
    bool preampEnabled = false;
    bool rbwAutoMode = true;
    double rbwHz = 10000.0;
    bool vbwSameAsRbw = true;
    double vbwHz = 10000.0;
    QString couplingMode = "AC";
    QString measurementMode = "NORMAL";
    double amplitudeDbm = 0.0;
    double resolutionHz = 1000.0;
};

// ============ 信号发生器相关 ============
struct SignalGeneratorConfig {
    double powerDbm = 0.0;
    bool frequencyCompensationEnabled = false;
    bool powerCompensationEnabled = false;
    QString outputConnector = "RF OUT";
    bool powerFlatteningEnabled = false;
    QString modulationMode = "OFF";
    bool outputEnabled = true;
};

// ============ 频谱仪底层配置 ============
struct SaConfiguration {
    double centerFreqHz = 0.0;
    double rbwHz = 0.0;
    double spanHz = 0.0;
    double refLevelDbm = 0.0;
    double vbwHz = 0.0;
    int sweepPoints = 0;

    // 📌 新增RF前端参数
    double attenuatorDb = 0.0;      // 衰减器 (0-70 dB)
    bool preampEnabled = false;      // 前置放大器 (ON/OFF)
    std::string couplingMode = "DC"; // AC/DC耦合模式
};

struct PeakMeasurement {
    double peakDbm;
    double peakFreqHz;
};

struct MeasurementData {
    int seqIndex;
    double freqHzSet;
    double powerDbmSet;
    double rbwHz;
    double vbwHz;
    double spanHz;
    double attenDb;
    int rep;
    std::chrono::system_clock::time_point timestamp;
    double peakDbm;
    double peakFreqHz;
    double phaseNoiseDbcPerHz;
    double refLevelDbm;
};

// ============ 方案管理系统 ============
struct TestScheme {
    QString name;  // 方案名称
    FrequencySweepConfig sweepConfig;
    SpectrumAnalyzerConfig saConfig;
    SignalGeneratorConfig sgConfig;
    // ✅ 新增：只要这两行
    QString schemeName;           // 方案名称（兼容方案管理）
    QString description = "";     // 方案描述
};

// ============ BRB诊断结果相关 ============
#include <QMap>
#include <QVariant>

// 频率范围
struct FrequencyRange {
    double min = 0.0;
    double max = 0.0;
};

// 系统级诊断结果
struct SystemDiagnosis {
    QMap<QString, double> probabilities;  // 中文 key：'正常','幅度失准','频率失准','参考电平失准'
    QString predictedClass;               // 中文字符串
    double maxProb = 0.0;                 // 0~1
    bool isNormal = false;
};

// BRB诊断完整结果
struct DiagnosisResult {
    QString inputFile;                    // 输入文件路径
    int dataPoints = 0;                   // 数据点数
    FrequencyRange frequencyRange;        // 频率范围
    QMap<QString, double> features;       // 特征值
    SystemDiagnosis systemDiagnosis;      // 系统级诊断
    QMap<QString, double> moduleDiagnosis;// 模块诊断概率 (0~1)
    QMap<QString, QVariant> evidence;     // 证据 (可选)
    
    // Ground Truth (仿真验收用)
    QString gtSystemFaultClass;           // 如 amp_error/freq_error/ref_error/normal
    QString gtModule;                     // 中文模块名
    bool hasGroundTruth = false;          // 是否有 GT
    bool matchResult = false;             // 验收是否通过
};
