# BRB诊断集成说明

## 概述

此文档说明QT程序如何调用BRB诊断模块的接口设计。

**重要说明：** 
- 本功能与QT程序中已有的 `viz-cli.exe` 可视化功能完全独立
- `viz-cli.exe` 用于显示系统结构图（原有功能）
- `brb_diagnosis.exe` 用于BRB诊断（新增功能）
- 两者使用不同的进程对象，互不干扰
- **目标机器不需要安装Python环境**，所有依赖已打包在exe中

## 文件位置

### QT部分
- **QT程序位置**: `E:\project\QT_project\FMFD`
- **打包后的EXE位置**: 应用程序目录/BRB/brb_diagnosis.exe

## 使用方式

### 生产环境（使用打包的EXE）

系统直接使用打包好的exe文件：
1. 菜单栏 → 操作 → 运行BRB诊断
2. 选择要诊断的频响数据CSV文件（格式：frequency,amplitude）
3. 系统会自动调用brb_diagnosis.exe进行诊断
4. 诊断结果会显示在日志区域，并保存到 `brb_diagnosis_result.json`

**优点：**
- 目标机器不需要安装Python环境
- 不需要安装任何Python依赖包
- 类似于viz-cli.exe，开箱即用
- 部署简单，只需复制exe文件

### 配置路径（可选）

如果需要自定义exe路径：
1. 菜单栏 → 配置 → BRB诊断路径配置
2. 设置BRB诊断EXE路径（默认：应用程序目录/BRB/brb_diagnosis.exe）

## 打包说明

### 将Python程序打包为exe

在Python程序目录下执行：
```bash
cd D:\PycharmProjects\FMFD\FMFD
pip install pyinstaller
pyinstaller --onefile brb_diagnosis_cli.py
```

将生成的exe文件复制到QT程序目录：
```bash
copy dist\brb_diagnosis_cli.exe <应用程序目录>\BRB\brb_diagnosis.exe
```

## BRB诊断程序接口说明

### 命令行参数

```bash
brb_diagnosis.exe --input <input_csv> --output <output_json> [OPTIONS]
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
- **菜单**: 配置 → BRB诊断路径配置
- **功能**: 配置BRB诊断EXE路径

### 诊断功能
- **菜单**: 操作 → 运行BRB诊断
- **功能**: 选择CSV文件进行BRB诊断
- **结果**: 
  - 在诊断日志区域显示系统级和模块级诊断结果
  - 保存完整JSON结果到 `brb_diagnosis_result.json`

### 相关类和方法

**FMFD.h:**
```cpp
// BRB诊断相关成员变量
QProcess* m_brbProc;
QString m_brbExePath;     // BRB诊断exe路径

// BRB诊断相关槽函数
void runBRBDiagnosis();                    // 执行诊断
void onBRBDiagnosisFinished(...);          // 诊断完成回调
void onBRBDiagnosisReadyRead();            // 读取输出
void configureBRBPaths();                   // 配置路径
```

## 打包依赖说明

### Python依赖（仅打包时需要）

打包exe时需要以下Python包，但**目标机器不需要安装**：
```bash
pip install numpy pandas scipy scikit-learn pyinstaller
```

**Required packages for packaging:**
- `numpy` - Numerical computing
- `pandas` - Data manipulation
- `scipy` - Scientific computing (signal processing)
- `scikit-learn` - Machine learning utilities
- `pyinstaller` - 用于打包exe

### Python程序结构（仅开发时参考）
- `FMFD/BRB/system_brb.py`: 系统级BRB推理
- `FMFD/BRB/module_brb.py`: 模块级BRB推理
- `FMFD/baseline/baseline.py`: 基线处理
- `FMFD/features/extract.py`: 特征提取

## 部署说明

1. 将 `brb_diagnosis.exe` 放置在应用程序目录下的 `BRB` 文件夹中
2. 确保exe文件具有执行权限
3. 目标机器不需要安装Python或任何Python包
4. 类似于viz-cli.exe的部署方式

## 版本历史

- **v2.0** (2024-12): 简化为仅使用exe，移除Python依赖
- **v1.0** (2024): 初始版本，支持Python脚本调用和EXE调用
