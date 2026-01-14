#pragma once
#include <string>
#include <vector>
#include "CommonTypes.h"  // 包含公共类型定义
#include <visa.h>


class InstrumentController
{
public:
    InstrumentController();
    ~InstrumentController();

    bool initializeInstruments(const std::string& sgResource,
        const std::string& saResource);
    void disconnect();
    bool isConnected() const;

    bool setSignalGenerator(double freqHz, double powerDbm, bool outputOn);
    bool configureSpectrumAnalyzer(const SaConfiguration& config);
    bool triggerSingleSweep();
    PeakMeasurement readMarkerPeak();
    double readReferenceLevel();
    std::vector<double> readTraceData();

    std::string getInstrumentInfo(bool isSA);
    // AC/DC耦合控制
    bool setInputCoupling(const std::string& mode); // 设置AC或DC耦合
    std::string getInputCoupling();                  // 查询当前耦合模式

private:
    ViSession m_defaultRM = VI_NULL;
    ViSession m_sgSession = VI_NULL;   // 信号源（如果是独立仪器）
    ViSession m_saSession = VI_NULL;   // 4082/4052 频谱仪

    bool writeCommand(ViSession session, const std::string& command);
    std::string queryCommand(ViSession session, const std::string& command);
    double parseResponseToDouble(const std::string& response);
};
