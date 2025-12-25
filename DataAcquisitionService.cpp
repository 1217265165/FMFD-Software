
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <cctype>
#include <cmath>
#include <vector>
#include <functional>
#include "CommonTypes.h"  // 包含公共类型定义
#include "DataAcquisitionService.h"
#include "InstrumentController.h"




// 字符串去空格工具
static std::string trim(const std::string& s) {
    auto front = std::find_if_not(s.begin(), s.end(), [](int c) {return std::isspace(c);});
    auto back = std::find_if_not(s.rbegin(), s.rend(), [](int c) {return std::isspace(c);}).base();
    return (front < back) ? std::string(front, back) : "";
}

// ============= 文件导入 =============
std::vector<double> DataAcquisitionService::loadFrequenciesFromFile(const std::string& filename) {
    std::vector<double> freq_list;
    std::ifstream file(filename);
    if (!file.is_open()) {
        logStatus("无法打开频点文件: " + filename);
        return freq_list;
    }
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        std::istringstream iss(line);
        double value = 0;
        std::string unit;
        iss >> value >> unit;
        if (unit.find("GHz") != std::string::npos)
            value *= 1e9;
        else if (unit.find("MHz") != std::string::npos)
            value *= 1e6;
        else if (unit.find("kHz") != std::string::npos)
            value *= 1e3;
        else if (unit.empty() && value < 1e5)
            value *= 1e3;
        if (value > 0)
            freq_list.push_back(value);
    }
    file.close();
    std::sort(freq_list.begin(), freq_list.end());
    freq_list.erase(std::unique(freq_list.begin(), freq_list.end()), freq_list.end());
    logStatus("从文件加载了 " + std::to_string(freq_list.size()) + " 个频点");
    return freq_list;
}

// ============= 分段自动生成 =============
struct FreqSegment {
    double start_Hz, stop_Hz, step_Hz;
};

std::vector<double> DataAcquisitionService::generateFrequenciesSegmented() {
    std::vector<FreqSegment> freq_segments = {
        {10e3, 1e6, 10e3},
        {1e6, 10e6, 1e6},
        {10e6, 500e6, 1e7},
        {500e6, 6.7e10, 50e6}
    };
    std::vector<double> frequencies;
    for (const auto& seg : freq_segments) {
        for (double f = seg.start_Hz; f <= seg.stop_Hz + seg.step_Hz / 2; f += seg.step_Hz)
            frequencies.push_back(f);
    }
    std::sort(frequencies.begin(), frequencies.end());
    frequencies.erase(std::unique(frequencies.begin(), frequencies.end()), frequencies.end());
    logStatus("生成了 " + std::to_string(frequencies.size()) + " 个默认频点（分段模式）");
    return frequencies;
}

std::vector<double> DataAcquisitionService::generateFrequenciesBySegments(const std::vector<FrequencySweepSegment>& segments) {
    std::vector<double> frequencies;
    for (const auto& seg : segments) {
        for (double f = seg.startHz; f <= seg.stopHz + seg.stepHz / 2; f += seg.stepHz)
            frequencies.push_back(f);
    }
    std::sort(frequencies.begin(), frequencies.end());
    frequencies.erase(std::unique(frequencies.begin(), frequencies.end()), frequencies.end());
    logStatus("生成了 " + std::to_string(frequencies.size()) + " 个频点（自定义分段模式）");
    return frequencies;
}

// ============= 构造/析构 =============
DataAcquisitionService::DataAcquisitionService()
    : m_controller(std::make_unique<InstrumentController>()), m_isMeasuring(false) {}

DataAcquisitionService::~DataAcquisitionService() {
    stopMeasurement();
    if (m_dataFile.is_open()) m_dataFile.close();
}

void DataAcquisitionService::setCallbacks(StatusCallback statusCb, DataCallback dataCb, FrequencyResponseCallback freqRespCb) {
    m_statusCallback = statusCb;
    m_dataCallback = dataCb;
    m_freqResponseCallback = freqRespCb;
}

// ============= 初始化 =============
bool DataAcquisitionService::initialize(const std::string& sgResource, const std::string& saResource) {
    bool success = m_controller->initializeInstruments(sgResource, saResource);
    if (success) logStatus("仪器初始化成功");
    else logStatus("仪器初始化失败");
    return success;
}

// ============= 核心：配置入口 =============
bool DataAcquisitionService::startFrequencySweepByConfig(const FrequencySweepConfig& config) {
    std::vector<double> freqs;

    if (config.mode == FrequencySweepConfig::FileImport) {
        logStatus("配置模式：文件导入");
        freqs = loadFrequenciesFromFile(config.freqFilePath);
        if (freqs.empty()) {
            logStatus("错误：频点文件为空或无法读取！");
            return false;
        }
    }
    else {
        logStatus("配置模式：分段生成");
        freqs = generateFrequenciesBySegments(config.segments);
        if (freqs.empty()) {
            logStatus("错误：分段配置未生成任何频点！");
            return false;
        }
    }

    // 使用默认采集参数（后续可接入 SpectrumAnalyzerConfig）
    double power = getDefaultPower();
    double rbw = getDefaultRbw();
    std::string vbw = getDefaultVbwMode();

    logStatus("开始采集：频点数=" + std::to_string(freqs.size()) +
        ", 功率=" + std::to_string(power) + " dBm" +
        ", RBW=" + std::to_string(rbw) + " Hz" +
        ", VBW模式=" + vbw);

    return startFrequencyResponseTest(freqs, power, rbw, vbw);

}

// ============= 自动采集入口 =============
bool DataAcquisitionService::startAutoFrequencyResponseTest() {
    auto freqs = getDefaultFrequencies();
    double power = getDefaultPower();
    double rbw = getDefaultRbw();
    std::string vbw = getDefaultVbwMode();
    return startFrequencyResponseTest(freqs, power, rbw, vbw);
}
std::vector<double> DataAcquisitionService::getDefaultFrequencies()
{
    // ============ 修改：使用分段生成默认频点，而不是等间距100个点 ============
    std::vector<FrequencySweepSegment> defaultSegments = {
        {10e3,    1e6,    10e3},      // 10 kHz - 1 MHz，步长10 kHz
        {1e6,     10e6,   1e6},     // 1 MHz - 10 MHz，步长1MHz
        {10e6,    500e6,  1e7},       // 10 MHz - 500 MHz，步长10 MHz
        {500e6,   6.7e10, 50e6},      // 500 MHz - 6.7 GHz，步长50 MHz
    };

    std::vector<double> frequencies;
    for (const auto& seg : defaultSegments) {
        for (double f = seg.startHz; f <= seg.stopHz + seg.stepHz / 2; f += seg.stepHz) {
            frequencies.push_back(f);
        }
    }

    std::sort(frequencies.begin(), frequencies.end());
    frequencies.erase(std::unique(frequencies.begin(), frequencies.end()), frequencies.end());

    return frequencies;
}


double DataAcquisitionService::getDefaultPower() { return 0.0; }
double DataAcquisitionService::getDefaultRbw() { return 10000.0; }
std::string DataAcquisitionService::getDefaultVbwMode() { return "same"; }

// ============= 基础采集入口 =============
bool DataAcquisitionService::startFrequencyResponseTest(const std::vector<double>& frequenciesHz,
    double powerDbm, double rbwHz, const std::string& vbwMode) {
    if (m_isMeasuring) {
        logStatus("采集已在进行中，请先停止当前采集");
        return false;
    }
    if (frequenciesHz.empty()) {
        logStatus("错误：频点列表为空");
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream filename;
    filename << "frequency_response_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << ".csv";

    m_dataFile.open(filename.str());
    if (!m_dataFile.is_open()) {
        logStatus("无法创建数据文件");
        return false;
    }
    m_dataFile << "Frequency(Hz),Frequency(MHz),Amplitude(dBm)\n";

    m_isMeasuring = true;
    m_measurementThread = std::thread(&DataAcquisitionService::frequencyResponseThreadFunction, this,
        frequenciesHz, powerDbm, rbwHz, vbwMode);

    logStatus("频响采集启动");
    return true;
}

void DataAcquisitionService::stopMeasurement() {
    m_isMeasuring = false;
    if (m_measurementThread.joinable()) m_measurementThread.join();
    if (m_dataFile.is_open()) m_dataFile.close();
    logStatus("采集已停止");
}

bool DataAcquisitionService::isMeasuring() const { return m_isMeasuring; }

// ============= 采集线程主函数 =============
void DataAcquisitionService::frequencyResponseThreadFunction(const std::vector<double>& frequenciesHz,
    double powerDbm, double rbwHz, const std::string& vbwMode) {
    double vbwToUse = rbwHz;
    double minFreq = *std::min_element(frequenciesHz.begin(), frequenciesHz.end());
    double maxFreq = *std::max_element(frequenciesHz.begin(), frequenciesHz.end());
    double freqRange = maxFreq - minFreq;
    double spanHz = std::max(freqRange * 0.1, rbwHz * 100);

    logStatus("开始频响采集，总计 " + std::to_string(frequenciesHz.size()) + " 个频点");

    for (size_t i = 0; i < frequenciesHz.size() && m_isMeasuring; ++i) {
        double freqHz = frequenciesHz[i];
        double dynamicSpan = spanHz;
        if (freqHz < 1e6) dynamicSpan = std::max(freqHz * 0.1, rbwHz * 10);
        double refLevel = powerDbm + 20;

        SaConfiguration saConfig{ freqHz, rbwHz, dynamicSpan, refLevel, vbwToUse, 1001 };
        if (!m_controller->configureSpectrumAnalyzer(saConfig)) {
            logStatus("频谱仪配置失败: " + std::to_string(freqHz) + " Hz");
            continue;
        }
        logStatus("频谱仪已配置: " + std::to_string(freqHz / 1e6) + " MHz");

        if (!m_controller->setSignalGenerator(freqHz, powerDbm, true)) {
            logStatus("信号源配置失败: " + std::to_string(freqHz) + " Hz");
            continue;
        }
        logStatus("信号源已配置: " + std::to_string(freqHz / 1e6) + " MHz");

        // ======================== 三阶段等待逻辑 ========================
        logStatus("等待信号源稳定(200ms)...");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        logStatus("等待频谱仪扫频完成(200ms)...");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        logStatus("等待测量数据稳定(100ms)...");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        logStatus("读取峰值: " + std::to_string(freqHz / 1e6) + " MHz");
        PeakMeasurement peak = m_controller->readMarkerPeak();
        logFrequencyResponse(freqHz, peak.peakDbm);

        if (m_dataFile.is_open()) {
            m_dataFile << freqHz << "," << freqHz / 1e6 << "," << peak.peakDbm << "\n";
            m_dataFile.flush();
        }

        if (i % 10 == 0 || i == frequenciesHz.size() - 1) {
            logStatus("进度: " + std::to_string(i + 1) + "/" +
                std::to_string(frequenciesHz.size()) + " (" +
                std::to_string((i + 1) * 100 / frequenciesHz.size()) + "%)");
        }

        // ==================== 频率跳跃检测 ====================
        double waitTimeMs = 200;
        if (i > 0) {
            double freqStep = std::abs(frequenciesHz[i] - frequenciesHz[i - 1]);
            if (freqStep > 100e6) {
                waitTimeMs = 300;
                logStatus("检测到大幅跳频: " + std::to_string(freqStep / 1e6) + " MHz，额外等待(300ms)...");
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(waitTimeMs)));
    }

    m_controller->setSignalGenerator(1e9, -100, false);
    m_isMeasuring = false;
    logStatus("频响采集完成");
}

// ============= 日志接口 =============
void DataAcquisitionService::logStatus(const std::string& message) {
    std::cout << "[STATUS] " << message << std::endl;
    if (m_statusCallback) m_statusCallback(message);
}

void DataAcquisitionService::logData(const MeasurementData& data) {
    std::stringstream ss;
    ss << "Data point: Frequency=" << data.freqHzSet / 1e6 << "MHz, Peak=" << data.peakDbm << "dBm";
    std::cout << "[DATA] " << ss.str() << std::endl;
    if (m_dataCallback) m_dataCallback(data);
}

void DataAcquisitionService::logFrequencyResponse(double freqHz, double amplitude) {
    std::stringstream ss;
    ss << "Frequency response: " << freqHz / 1e6 << " MHz = " << amplitude << " dBm";
    std::cout << "[FREQ_RESP] " << ss.str() << std::endl;
    if (m_freqResponseCallback) m_freqResponseCallback(freqHz, amplitude);
}
