
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
