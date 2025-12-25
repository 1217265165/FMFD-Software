#pragma once
#include <QObject>
#include <QVector>
#include <QMap>
#include <QStringList>

// 系统级异常类型
enum class AnomalyType {
    Normal = 0,           // 正常
    AmplitudeMisalignment,  // 幅度失准
    FrequencyMisalignment,  // 频率失准
    RefLevelMisalignment    // 参考电平失准
};

// BRB规则结构
struct HierarchicalBRBRule {
    QVector<double> attributeMatch;  // 属性匹配度
    QVector<double> attributeWeights; // 属性权重
    QMap<QString, double> belief;     // 置信度分配
    double ruleWeight = 1.0;          // 规则权重
};

// 系统级输出结构
struct SystemLevelOutput {
    double amplitudeConfidence = 0.0;   // 幅度失准置信度
    double frequencyConfidence = 0.0;   // 频率失准置信度
    double refLevelConfidence = 0.0;    // 参考电平失准置信度
    double uncertainty = 0.0;           // 不确定性
    AnomalyType dominantType = AnomalyType::Normal;
};

// 模块级输出结构
struct ModuleLevelOutput {
    QMap<QString, double> moduleFaultProb; // 模块故障概率
    double uncertainty = 0.0;               // 不确定性
};

// 分层BRB诊断引擎
class HierarchicalBRBEngine : public QObject
{
    Q_OBJECT
public:
    explicit HierarchicalBRBEngine(QObject* parent = nullptr);
    
    // 系统级推理
    SystemLevelOutput systemLevelInference(const QMap<QString, double>& globalFeatures);
    
    // 模块级推理
    ModuleLevelOutput moduleLevelInference(const SystemLevelOutput& systemOutput,
                                          const QMap<QString, double>& detailedFeatures);
    
    // 两层联合推理
    void hierarchicalInference(const QMap<QString, double>& globalFeatures,
                              const QMap<QString, double>& detailedFeatures);
    
    // 添加系统级规则
    void addSystemLevelRule(const HierarchicalBRBRule& rule);
    
    // 添加模块级规则
    void addModuleLevelRule(AnomalyType type, const HierarchicalBRBRule& rule);
    
    // 获取模块名称列表
    QStringList moduleNames() const { return m_modules; }
    
    // 参数更新（反馈修正机制）
    void updateRuleWeights(const QString& confirmedModule, double learningRate = 0.1);

signals:
    void systemLevelDiagnosisReady(const SystemLevelOutput& output);
    void moduleLevelDiagnosisReady(const ModuleLevelOutput& output);
    void logMessage(const QString& msg);

private:
    // 属性匹配度计算（线性插值）
    double calculateAttributeMatch(double inputValue, double refLow, double refHigh);
    
    // 规则激活权重计算
    double calculateRuleActivation(const HierarchicalBRBRule& rule,
                                   const QMap<QString, double>& features,
                                   const QStringList& featureOrder);
    
    // 证据推理合成
    QMap<QString, double> evidenceReasoning(const QVector<HierarchicalBRBRule>& rules,
                                           const QMap<QString, double>& features,
                                           const QStringList& featureOrder,
                                           double& uncertainty);
    
    // 根据系统级输出选择特征子集
    QStringList selectFeatureSubset(AnomalyType type);
    
    // 根据系统级输出选择规则子库
    QVector<HierarchicalBRBRule> selectRuleSubset(AnomalyType type);
    
    // 初始化规则库
    void initializeSystemLevelRules();
    void initializeModuleLevelRules();
    
    // 数据成员
    QVector<HierarchicalBRBRule> m_systemLevelRules;
    QMap<AnomalyType, QVector<HierarchicalBRBRule>> m_moduleLevelRules;
    
    QStringList m_modules;
    QStringList m_systemLevelFeatures;
    
    // 特征-异常类型映射
    QMap<AnomalyType, QStringList> m_featureMapping;
    
    // 上次诊断结果（用于反馈修正）
    SystemLevelOutput m_lastSystemOutput;
    QString m_lastPredictedModule;
};
