#pragma once
#include "InstrumentController.h"
#include "CommonTypes.h"
#include <fstream>
#include <memory>
#include <thread>
#include <vector>
#include <string>
#include <functional>

class DataAcquisitionService {
public:
    using StatusCallback = std::function<void(const std::string&)>;
    using DataCallback = std::function<void(const MeasurementData&)>;
    using FrequencyResponseCallback = std::function<void(double, double)>;
    using CompletionCallback = std::function<void()>;

    DataAcquisitionService();
    ~DataAcquisitionService();

    void setCallbacks(StatusCallback statusCb, DataCallback dataCb, FrequencyResponseCallback freqRespCb);
    void setCompletionCallback(CompletionCallback completionCb);
    bool initialize(const std::string& sgResource, const std::string& saResource);

    // ============= 采集入口（三种方式） =============
    // 方式1：配置入口（推荐）
    bool startFrequencySweepByConfig(const FrequencySweepConfig& config);

    // 方式2：自定义参数入口
    bool startFrequencyResponseTest(const std::vector<double>& frequenciesHz,
        double powerDbm, double rbwHz, const std::string& vbwMode);

    // 方式3：一键自动采集入口
    bool startAutoFrequencyResponseTest();

    void stopMeasurement();
    bool isMeasuring() const;

    // ============= 频点工具接口 =============
    std::vector<double> loadFrequenciesFromFile(const std::string& filename);
    std::vector<double> generateFrequenciesSegmented();
    std::vector<double> generateFrequenciesBySegments(const std::vector<FrequencySweepSegment>& segments);

    // ============= 日志接口 =============
    void logStatus(const std::string& message);

private:
    // ============= 默认参数生成 =============
    std::vector<double> getDefaultFrequencies();
    double getDefaultPower();
    double getDefaultRbw();
    std::string getDefaultVbwMode();

    // ============= 采集线程 =============
    void frequencyResponseThreadFunction(const std::vector<double>& frequenciesHz,
        double powerDbm, double rbwHz, const std::string& vbwMode);

    // ============= 日志输出 =============
    void logData(const MeasurementData& data);
    void logFrequencyResponse(double freqHz, double amplitude);

    // ============= 成员变量 =============
    std::unique_ptr<InstrumentController> m_controller;
    std::ofstream m_dataFile;
    std::thread m_measurementThread;
    bool m_isMeasuring = false;

    StatusCallback m_statusCallback;
    DataCallback m_dataCallback;
    FrequencyResponseCallback m_freqResponseCallback;
    CompletionCallback m_completionCallback;
};
