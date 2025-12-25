#include "InstrumentController.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <cmath>
#include <cstdlib>

#define SIMULATION_MODE 0  // 0 = 真机模式

InstrumentController::InstrumentController() {}

InstrumentController::~InstrumentController()
{
    disconnect();
}

bool InstrumentController::initializeInstruments(const std::string& sgResource,
    const std::string& saResource)
{
#if SIMULATION_MODE
    std::cout << "[SIM] 初始化仪器 SG=" << sgResource
        << " SA=" << saResource << std::endl;
    return true;
#else
    ViStatus status;

    status = viOpenDefaultRM(&m_defaultRM);
    if (status != VI_SUCCESS) {
        std::cerr << "viOpenDefaultRM 失败, status=" << status << std::endl;
        return false;
    }

    // 打开频谱仪 4082/4052
    status = viOpen(m_defaultRM,
        (ViRsrc)saResource.c_str(),
        VI_NULL, VI_NULL,
        &m_saSession);
    if (status != VI_SUCCESS) {
        std::cerr << "打开频谱仪失败, status=" << status << std::endl;
        disconnect();
        return false;
    }

    // 如有独立信号源，再打开；如果直接用 4082 内置信号源，可暂时不使用 m_sgSession
    if (!sgResource.empty()) {
        status = viOpen(m_defaultRM,
            (ViRsrc)sgResource.c_str(),
            VI_NULL, VI_NULL,
            &m_sgSession);
        if (status != VI_SUCCESS) {
            std::cerr << "打开信号源失败, status=" << status << std::endl;
            // 不强制失败，看你实际需要
        }
    }

    std::cout << "SA *IDN?: " << queryCommand(m_saSession, "*IDN?") << std::endl;
    if (m_sgSession)
        std::cout << "SG *IDN?: " << queryCommand(m_sgSession, "*IDN?") << std::endl;

    // 进入扫频分析功能 SweptSA0（按手册 :INST:CRE + :INST:SEL）[file:240]
    writeCommand(m_saSession, ":INST:CRE \"SweptSA\",\"SweptSA0\"");
    queryCommand(m_saSession, "*OPC?");
    writeCommand(m_saSession, ":INST:SEL \"SweptSA0\"");

    return true;
#endif
}

void InstrumentController::disconnect()
{
#if !SIMULATION_MODE
    if (m_sgSession) {
        viClose(m_sgSession);
        m_sgSession = VI_NULL;
    }
    if (m_saSession) {
        viClose(m_saSession);
        m_saSession = VI_NULL;
    }
    if (m_defaultRM) {
        viClose(m_defaultRM);
        m_defaultRM = VI_NULL;
    }
#endif
}

bool InstrumentController::isConnected() const
{
#if SIMULATION_MODE
    return true;
#else
    return m_saSession != VI_NULL;
#endif
}

//==================== 信号源（独立 SG） ====================

bool InstrumentController::setSignalGenerator(double freqHz,
    double powerDbm,
    bool outputOn)
{
#if SIMULATION_MODE
    std::cout << "[SIM] SG FREQ=" << freqHz
        << "Hz POW=" << powerDbm
        << "dBm OUT=" << (outputOn ? "ON" : "OFF") << std::endl;
    return true;
#else
    if (!m_sgSession) return false;

    // 下面命令需要根据你的思仪信号源程控手册调整
    if (!writeCommand(m_sgSession, "FREQ " + std::to_string(freqHz)))
        return false;
    if (!writeCommand(m_sgSession, "POW " + std::to_string(powerDbm)))
        return false;
    if (!writeCommand(m_sgSession, std::string("OUTP ") + (outputOn ? "ON" : "OFF")))
        return false;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return true;
#endif
}

//==================== 频谱仪 4082/4052 ====================

bool InstrumentController::configureSpectrumAnalyzer(const SaConfiguration& c)
{
#if SIMULATION_MODE
    std::cout << "[SIM] SA CF=" << c.centerFreqHz
        << "Hz SPAN=" << c.spanHz
        << "Hz RBW=" << c.rbwHz
        << "Hz VBW=" << c.vbwHz
        << "Hz RLEV=" << c.refLevelDbm << "dBm"
        << " ATT=" << c.attenuatorDb << "dB"
        << " PREAMP=" << (c.preampEnabled ? "ON" : "OFF")
        << " COUPL=" << c.couplingMode << "\n";
    return true;
#else
    if (!m_saSession) return false;

    // 以扫频分析 SweptSA 功能为例，命令名参考手册通用 SCPI 章节[file:240]
    if (!writeCommand(m_saSession, ":FREQ:CENT " + std::to_string(c.centerFreqHz)))
        return false;
    if (c.spanHz > 0 &&
        !writeCommand(m_saSession, ":FREQ:SPAN " + std::to_string(c.spanHz)))
        return false;

    if (c.rbwHz > 0 &&
        !writeCommand(m_saSession, ":BAND " + std::to_string(c.rbwHz)))
        return false;

    if (c.vbwHz > 0 &&
        !writeCommand(m_saSession, ":BAND:VID " + std::to_string(c.vbwHz)))
        return false;

    // ==================== 衰减器 ====================
    if (c.attenuatorDb >= 0 && c.attenuatorDb <= 70 &&
        !writeCommand(m_saSession, ":INP:ATT " + std::to_string(c.attenuatorDb)))
        return false;

    // ==================== 前置放大器 ====================
    if (!writeCommand(m_saSession,
        std::string(":INP:GAIN:STAT ") + (c.preampEnabled ? "ON" : "OFF")))
        return false;

    // ==================== AC/DC耦合 ====================
    if (!writeCommand(m_saSession, ":INP:COUP " + c.couplingMode))
        return false;

    if (!writeCommand(m_saSession,
        ":DISP:WIND:TRAC:Y:SCAL:RLEV "
        + std::to_string(c.refLevelDbm)))
        return false;


    if (c.sweepPoints > 0 &&
        !writeCommand(m_saSession, ":SWE:POIN " + std::to_string(c.sweepPoints)))
        return false;

    // 单次扫频模式
    writeCommand(m_saSession, ":INIT:CONT OFF");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return true;
#endif
}



bool InstrumentController::setInputCoupling(const std::string& mode) {
#if SIMULATION_MODE
    std::cout << "[SIM] 设置输入耦合方式: " << mode << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return true;
#else
    // 真实仪器VISA控制代码
    if (mode != "AC" && mode != "DC") {
        std::cerr << "错误: 耦合模式必须是 'AC' 或 'DC'" << std::endl;
        return false;
    }

    std::string command = "INP:COUP " + mode;
    return writeCommand(m_saSession, command);
#endif
}

std::string InstrumentController::getInputCoupling() {
#if SIMULATION_MODE
    std::string mode = "DC"; // 默认模拟DC耦合
    std::cout << "[SIM] 查询输入耦合方式: " << mode << std::endl;
    return mode;
#else
    // 真实仪器VISA查询代码
    std::string response = queryCommand(m_saSession, "INP:COUP?");

    // 清理响应字符串（移除换行符和空格）
    response.erase(std::remove_if(response.begin(), response.end(),
        [](unsigned char c) { return std::isspace(c); }),
        response.end());

    // 转换为大写
    std::transform(response.begin(), response.end(),
        response.begin(), ::toupper);

    if (response.find("AC") != std::string::npos) {
        return "AC";
    }
    else if (response.find("DC") != std::string::npos) {
        return "DC";
    }

    std::cerr << "警告: 无法识别耦合模式响应: " << response << std::endl;
    return "DC";
#endif
}


bool InstrumentController::triggerSingleSweep()
{
#if SIMULATION_MODE
    std::cout << "[SIM] SA single sweep\n";
    return true;
#else
    if (!m_saSession) return false;
    if (!writeCommand(m_saSession, ":INIT")) return false;
    // *OPC? 等待扫频结束
    std::string opc = queryCommand(m_saSession, "*OPC?");
    (void)opc;
    return true;
#endif
}

PeakMeasurement InstrumentController::readMarkerPeak()
{
    PeakMeasurement m{ 0.0, 0.0 };
#if SIMULATION_MODE
    // 这里保留你原来的模拟逻辑
    return m;
#else
    if (!m_saSession) return m;

    if (!triggerSingleSweep())
        return m;

    // 设定标记到最大值并读取 X/Y
    if (!writeCommand(m_saSession, ":CALC:MARK1:MAX"))
        return m;

    std::string y = queryCommand(m_saSession, ":CALC:MARK1:Y?");
    std::string x = queryCommand(m_saSession, ":CALC:MARK1:X?");

    m.peakDbm = parseResponseToDouble(y);
    m.peakFreqHz = parseResponseToDouble(x);
    return m;
#endif
}

double InstrumentController::readReferenceLevel()
{
#if SIMULATION_MODE
    return 0.0;
#else
    if (!m_saSession) return 0.0;
    std::string rsp = queryCommand(m_saSession,
        ":DISP:WIND:TRAC:Y:SCAL:RLEV?");
    return parseResponseToDouble(rsp);
#endif
}

std::vector<double> InstrumentController::readTraceData()
{
    std::vector<double> traceData;
#if SIMULATION_MODE
    // 模拟模式：返回一些模拟的轨迹数据
    int numPoints = 601; // 默认扫描点数
    traceData.reserve(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        // 生成模拟的频谱数据（简单的衰减曲线加噪声）
        double value = -80.0 + 10.0 * std::sin(i * 0.01) + (rand() % 20 - 10) * 0.1;
        traceData.push_back(value);
    }
    std::cout << "[SIM] readTraceData: 返回 " << traceData.size() << " 个数据点\n";
    return traceData;
#else
    if (!m_saSession) return traceData;

    // 触发单次扫描并等待完成
    if (!triggerSingleSweep())
        return traceData;

    // 读取轨迹数据
    // 对于 4082/4052 系列频谱仪，使用 :TRACE:DATA? 或 :FETCH:SAN? 命令读取轨迹数据
    std::string response = queryCommand(m_saSession, ":TRACE:DATA? TRACE1");

    if (response.empty()) {
        std::cerr << "读取轨迹数据失败：响应为空\n";
        return traceData;
    }

    // 解析响应数据
    // SCPI 仪器通常以逗号或空格分隔的ASCII格式返回数据
    std::istringstream iss(response);
    std::string token;

    while (std::getline(iss, token, ',')) {
        // 去除前后空格
        token.erase(0, token.find_first_not_of(" \t\r\n"));
        token.erase(token.find_last_not_of(" \t\r\n") + 1);

        if (!token.empty()) {
            try {
                double value = std::stod(token);
                traceData.push_back(value);
            }
            catch (const std::exception& e) {
                std::cerr << "解析轨迹数据时出错: " << e.what()
                    << " token: " << token << std::endl;
            }
        }
    }

    std::cout << "成功读取 " << traceData.size() << " 个轨迹数据点\n";
    return traceData;
#endif
}

//==================== VISA 辅助 ====================

bool InstrumentController::writeCommand(ViSession session,
    const std::string& command)
{
#if !SIMULATION_MODE
    if (!session) return false;
    ViStatus status;
    ViUInt32 retCount;
    std::string cmd = command + "\n";
    status = viWrite(session,
        (ViBuf)cmd.c_str(),
        (ViUInt32)cmd.size(),
        &retCount);
    if (status != VI_SUCCESS) {
        std::cerr << "viWrite 失败, status=" << status
            << " cmd=" << command << std::endl;
        return false;
    }
#endif
    return true;
}

std::string InstrumentController::queryCommand(ViSession session,
    const std::string& command)
{
#if !SIMULATION_MODE
    if (!session) return "";
    ViStatus status;
    ViUInt32 retCount;
    char buf[4096] = { 0 };

    std::string cmd = command + "\n";
    status = viWrite(session,
        (ViBuf)cmd.c_str(),
        (ViUInt32)cmd.size(),
        &retCount);
    if (status != VI_SUCCESS) {
        std::cerr << "viWrite(query) 失败, status=" << status << std::endl;
        return "";
    }

    status = viRead(session, (ViBuf)buf, sizeof(buf) - 1, &retCount);
    if (status != VI_SUCCESS) {
        std::cerr << "viRead 失败, status=" << status << std::endl;
        return "";
    }
    buf[retCount] = '\0';
    return std::string(buf);
#else
    return "1.0";
#endif
}

double InstrumentController::parseResponseToDouble(const std::string& response)
{
    try {
        return std::stod(response);
    }
    catch (...) {
        return 0.0;
    }
}

std::string InstrumentController::getInstrumentInfo(bool isSA)
{
#if SIMULATION_MODE
    return isSA ? "SIM SA" : "SIM SG";
#else
    ViSession s = isSA ? m_saSession : m_sgSession;
    if (!s) return "";
    return queryCommand(s, "*IDN?");
#endif
}




//ViStatus InstrumentController::setInputCoupling(const QString& mode)
//{
//    if (!isConnected()) return VI_ERROR_INV_SESSION;
//
//    QString command = "INPut:COUPling " + mode;
//    ViStatus status = viWrite(m_session,
//        (ViBuf)command.toLatin1().data(),
//        command.length(),
//        nullptr);
//    return status;
//}
//
//ViStatus InstrumentController::getInputCoupling(QString& mode)
//{
//    if (!isConnected()) return VI_ERROR_INV_SESSION;
//
//    char rdBuf[20] = { 0 };
//    long retCnt = 0;
//
//    ViStatus status = viWrite(m_session, "INPut:COUPling?", 15, &retCnt);
//    if (status != VI_SUCCESS) return status;
//
//    status = viRead(m_session, (ViBuf)rdBuf, 20, &retCnt);
//    mode = QString::fromLatin1(rdBuf, retCnt);
//    return status;
//}
