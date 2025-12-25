# BRB诊断Python集成说明

## 概述

此文档说明QT程序如何调用Python BRB诊断模块的接口设计。

**重要说明：** 
- 本功能与QT程序中已有的 `viz-cli.exe` 可视化功能完全独立
- `viz-cli.exe` 用于显示系统结构图（原有功能）
- `brb_diagnosis.exe` 用于BRB诊断（新增功能）
- 两者使用不同的进程对象，互不干扰

## 文件位置

### Python部分
- **Python程序位置**: `D:\PycharmProjects\FMFD\FMFD`
- **BRB诊断脚本**: `D:\PycharmProjects\FMFD\FMFD\brb_diagnosis_cli.py`

### QT部分
- **QT程序位置**: `E:\project\QT_project\FMFD`
- **打包后的EXE位置**: `E:\project\QT_project\FMFD\x64\Release\brb_diagnosis.exe`

## 使用方式

### 1. 当前开发阶段（使用Python脚本）

在QT程序中配置Python路径：
1. 菜单栏 → 配置 → BRB Python路径配置
2. 设置以下路径：
   - Python解释器路径: `python` 或 `python3` 或完整路径（如：`C:\Python39\python.exe`）
   - Python脚本路径: `D:\PycharmProjects\FMFD\FMFD\brb_diagnosis_cli.py`
   - 打包EXE路径: `E:\project\QT_project\FMFD\x64\Release\brb_diagnosis.exe`（暂时不存在）

运行BRB诊断：
1. 菜单栏 → 操作 → 运行BRB诊断 (Python)
2. 选择要诊断的频响数据CSV文件（格式：frequency,amplitude）
3. 系统会自动调用Python脚本进行诊断
4. 诊断结果会显示在日志区域，并保存到 `brb_diagnosis_result.json`

### 2. 未来生产阶段（使用打包的EXE）

将Python程序打包为exe：
```bash
# 在Python程序目录下执行
cd D:\PycharmProjects\FMFD\FMFD
pip install pyinstaller
pyinstaller --onefile brb_diagnosis_cli.py
```

将生成的exe文件复制到QT程序目录：
```bash
copy dist\brb_diagnosis_cli.exe E:\project\QT_project\FMFD\x64\Release\brb_diagnosis.exe
```

系统会自动检测：
- 如果 `brb_diagnosis.exe` 存在，则优先使用exe
- 如果 `brb_diagnosis.exe` 不存在，则使用Python脚本

## Python脚本接口说明

### 命令行参数

```bash
python brb_diagnosis_cli.py --input <input_csv> --output <output_json> [OPTIONS]
```

**必需参数：**
- `--input, -i`: 输入CSV文件路径（格式：frequency,amplitude）
- `--output, -o`: 输出JSON文件路径

**可选参数：**
- `--baseline, -b`: 基线数据目录（默认使用程序内置路径）
- `--mode, -m`: BRB推理模式，可选 `er`（增强版，默认）或 `simple`（简化版）
- `--verbose, -v`: 显示详细输出

### 输入格式

CSV文件，两列数据：
```csv
frequency,amplitude
1000000,10.5
1100000,10.8
1200000,11.2
...
```

### 输出格式

JSON文件，包含完整的诊断结果：
```json
{
  "status": "success",
  "input_file": "path/to/input.csv",
  "data_points": 1000,
  "frequency_range": {
    "min": 1000000.0,
    "max": 67000000000.0
  },
  "features": {
    "bias": 0.123,
    "gain": 1.05,
    "df": 12345.67,
    ...
  },
  "system_diagnosis": {
    "幅度失准": 0.45,
    "频率失准": 0.30,
    "参考电平失准": 0.25
  },
  "module_diagnosis": {
    "衰减器": 0.18,
    "前置放大器": 0.15,
    "时钟振荡器": 0.12,
    ...
  },
  "mode": "er"
}
```

## QT程序接口

### 配置界面
- **菜单**: 配置 → BRB Python路径配置
- **功能**: 配置Python解释器、脚本和EXE路径

### 诊断功能
- **菜单**: 操作 → 运行BRB诊断 (Python)
- **功能**: 选择CSV文件进行BRB诊断
- **结果**: 
  - 在诊断日志区域显示系统级和模块级诊断结果
  - 保存完整JSON结果到 `brb_diagnosis_result.json`

### 相关类和方法

**FMFD.h:**
```cpp
// BRB诊断相关成员变量
QProcess* m_brbProc;
QString m_brbPythonPath;  // Python解释器路径
QString m_brbScriptPath;  // Python脚本路径
QString m_brbExePath;     // 打包后的exe路径

// BRB诊断相关槽函数
void runBRBDiagnosis();                    // 执行诊断
void onBRBDiagnosisFinished(...);          // 诊断完成回调
void onBRBDiagnosisReadyRead();            // 读取输出
void configureBRBPaths();                   // 配置路径
```

## 依赖说明

### Python依赖
```bash
pip install numpy pandas scipy scikit-learn
```

**Required packages:**
- `numpy` - Numerical computing
- `pandas` - Data manipulation
- `scipy` - Scientific computing (signal processing)
- `scikit-learn` - Machine learning utilities

### Python程序结构
- `FMFD/BRB/system_brb.py`: 系统级BRB推理
- `FMFD/BRB/module_brb.py`: 模块级BRB推理
- `FMFD/baseline/baseline.py`: 基线处理
- `FMFD/features/extract.py`: 特征提取

## 版本历史

- **v1.0** (2024): 初始版本，支持Python脚本调用和EXE调用
