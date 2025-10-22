# 🧩 Multi-Sensor Logger & Analyzer (C++ + Python gRPC Practice)

## 🇬🇧 English Version

### Overview
This project demonstrates an end-to-end data pipeline for multi-sensor data logging and analysis.  
It consists of a **C++ logger** that simulates concurrent sensor data generation, and a **Python analyzer** that parses and evaluates the generated logs.  
This project is designed as a foundational step toward future **gRPC streaming** integration between C++ (server) and Python (client).

---

### Features

#### 🧠 C++ Module (`cpp-grpc-lab`)
- Simulates multiple sensors (temperature, humidity, pressure, general) using **multi-threading**.
- Logs sensor data as structured JSONL records with timestamps, thread IDs, and sequence numbers.
- Supports random delay (`std::this_thread::sleep_for`) for realistic asynchronous behavior.
- Thread-safe logger (using `std::mutex`) with optional lock toggle.
- Output example:
  ```json
  {"timestamp":"2025-10-15 16:21:01","sensor":"humidity-3","value":44.28,"thread_id":"22988","global_seq":0,"local_seq":0}
  ```

#### 🐍 Python Module (`pythonGrpcParser`)
- **parser.py** — Reads and validates JSONL logs into `SensorRow` objects.
- **rules.py** — Defines threshold, gap, and order validation logic.
- **report.py** — Generates a summary report, supports CLI arguments, and exports CSV files.
- Provides **colorized console output** for event visualization.

Example output:
```
[INFO] loaded rows: 100
=== SUMMARY ===
By kind:  {'THRESHOLD': 10}
By sensor (top 5):  {'humidity-3': 3, 'temperature-1': 2, ...}
```

CSV export example:
```csv
kind,sensor,message,timestamp,value,thread_id,global_seq,local_seq
THRESHOLD,humidity-3,value 69.07 > 65.00,2025-10-21 05:25:19.182,69.07,23148,0,0
```

---

### Usage

#### Run the C++ logger
```
> ./cpp-grpc-lab.exe
```
Generates file:
```
sample/multi_sensor_log.jsonl
```

#### Run the Python analyzer
```
> python src/report.py sample/multi_sensor_log.jsonl --output result/report.csv
```

#### Output
- Console summary (colored)
- CSV report saved in `result/`

---

### Future Plan
- Integrate **gRPC bidirectional streaming**  
  → C++ server continuously streams sensor logs  
  → Python client performs real-time rule analysis  

---

## 🇹🇼 中文版本

### 專案簡介
這是一個從資料產生到分析的完整示範專案。  
由 **C++** 模擬多感測器並行輸出資料（JSONL 格式），  
再由 **Python** 解析與分析結果，最終匯出報表。  
本專案是未來 **gRPC 即時串流整合** 的基礎練習。

---

### 功能說明

#### 🧩 C++ 模組 (`cpp-grpc-lab`)
- 使用多執行緒模擬多感測器資料（溫度、濕度、壓力等）
- 以 JSONL 格式輸出時間戳記、執行緒 ID、序號與測值
- 支援亂序與延遲模擬（sleep 隨機毫秒）
- 提供 thread-safe Logger，可切換是否加鎖

輸出範例：
```json
{"timestamp":"2025-10-15 16:21:01","sensor":"pressure-6","value":101.68,"thread_id":"37104","global_seq":3,"local_seq":1}
```

#### 🐍 Python 模組 (`pythonGrpcParser`)
- **parser.py**：解析 JSONL 並建立 SensorRow 物件
- **rules.py**：判斷閾值超標、資料跳動、亂序等規則
- **report.py**：統計事件數據、輸出彩色報表與 CSV 檔案

範例輸出：
```
=== FIRST 10 EVENTS ===
THRESHOLD  humidity-4  message value 69.38 > 65.00  value 69.38  ts=2025-10-21 05:25:19
```

---

### 使用方式
1️⃣ 執行 C++ 產生資料  
```
./cpp-grpc-lab.exe
```

2️⃣ 執行 Python 分析  
```
python src/report.py sample/multi_sensor_log.jsonl --output result/report.csv
```

3️⃣ 結果輸出  
- 終端彩色摘要報告  
- `result/report.csv` 匯出檔案

---

### 後續規劃
- 實作 **C++ ↔ Python gRPC 即時串流**  
- 加入 Web 介面即時視覺化（FastAPI + WebSocket）  
- 改進感測器異常分析與警示模型  