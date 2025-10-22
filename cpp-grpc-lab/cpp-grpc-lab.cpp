// cpp-grpc-lab.cpp: 定義應用程式的進入點。
//

#include "cpp-grpc-lab.h"
#include "include/logger.h"
#include "include/sensor_types.h"

using namespace std;

// 感測器資料結構
// 類似資料封包
struct SensorData {
	double temperature;
	double humidity;
	double pressure;
	string timestamp;
	bool alert = false; // 是否觸發警告
	string alert_reason; // 警告原因
};

// 取得現在時間 (ISO 8601 格式)
string get_currentTime() {
	// step 1 : 取得系統時鐘的當前時間點 對應電腦的UTC 時間或作業系統時間
	// auto 可以讓程式自動推斷這個複雜的型別
	auto now = chrono::system_clock::now();
	// step 2 : time_t (整數秒, ex: 1728403721)
	// 將now() 回傳的時間戳，轉成可讀取的整數時間(time_t)
	time_t now_time = chrono::system_clock::to_time_t(now);

	// step 3: tm 結構（年月日時分秒）{year=2025, month=10, day=8, ...}
	tm local_tm{}; // 是C 語言的時間結構，包含年月日時分秒欄位

	#ifdef _WIN32
		localtime_s(&local_tm, &now_time);
	#else
		localtime_r(&now_time, &local_tm);
	#endif

	// step 4: 格式化成字串 "2025-10-08T22:28:41"
	// 把人類可讀的時間結構（tm）格式化成字串
	ostringstream oss;
	oss << put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
	oss << "." << setw(3) << setfill('0')
		<< chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
	return oss.str();
}

double read_fake_temperature()
{
	// 產生一個介於 20.0 到 30.0 之間的隨機浮點數
	static random_device rd;  // 用於取得隨機數種子
	static mt19937 gen(rd()); // 使用 Mersenne Twister 引擎
	static uniform_real_distribution<> dist(36.0, 42.0);
	return dist(gen);
}

// 模擬感測器數據
// 可控制上下限
double random_value(double min, double max) {
	static random_device rd;
	static mt19937 gen(rd());
	uniform_real_distribution<> dist(min, max);
	return dist(gen);
}

SensorData read_all_sensors()
{
	SensorData data;
	data.timestamp = get_currentTime();
	data.temperature = random_value(35.5, 42.5);
	data.humidity = random_value(38.0, 70.0);
	data.pressure = random_value(99.0, 102.5);
	return data;
}

// 閥值檢查 (超過或低於即觸發)
void check_thresholds(SensorData& d, const map<string, double>& upper,
	const map<string, double>& lower) {

	// it->first 表示 key, it->second 表示 value 
	/*
		auto it = upper.find("temperature")，它會回傳
		如果key 存在 -> 回傳指向該元素的iterator
		如果key 不存在 -> 回傳upper.end()
	
	*/
	// 高於上限，寫法結合 具初始化的if (if with initializer)
	if (auto it = upper.find("temperature"); it != upper.end() && d.temperature > it->second) {
		d.alert = true;
		d.alert_reason += "temperature>";
		d.alert_reason += to_string(it->second )+ ";";
	}
	if (auto it = upper.find("humidity"); it != upper.end() && d.humidity > it->second) {
		d.alert = true;
		d.alert_reason += "humidity>";
		d.alert_reason += to_string(it->second) + ";";
	}
	if (auto it = upper.find("pressure"); it != upper.end() && d.pressure > it->second) {
		d.alert = true;
		d.alert_reason += "pressure>";
		d.alert_reason += to_string(it->second) + ";";
	}

	// 低於下限
	if (auto it = lower.find("temperature"); it != lower.end() && d.temperature < it->second) {
		d.alert = true;
		d.alert_reason += "temperature<";
		d.alert_reason += to_string(it->second) + ";";
	}
	if (auto it = lower.find("humidity"); it != lower.end() && d.humidity < it->second) {
		d.alert = true;
		d.alert_reason += "humidity<";
		d.alert_reason += to_string(it->second) + ";";
	}
	if (auto it = lower.find("pressure"); it != lower.end() && d.pressure < it->second) {
		d.alert = true;
		d.alert_reason += "pressure<";
		d.alert_reason += to_string(it->second) + ";";
	}
}


// 將 SensorData 轉成JSON 字串
string to_json(const SensorData& data) {
	ostringstream oss;
	oss << fixed << setprecision(2);
	oss << "{"
		<< "\"timestamp\":" << data.timestamp << ", "
		<< "\"temperature\":" << data.temperature << ", "
		<< "\"humidity\":" << data.humidity << ", "
		<< "\"pressure\":" << data.pressure
		<< "}";

	return oss.str();
}

void lesson2() {
	cout << "[cpp-core][lesson2] multi-sensor demo started!\n";
	for (int i = 0; i < 5; ++i) {
		SensorData data = read_all_sensors();
		string json_str = to_json(data);
		cout << json_str << endl;
		//cout << fixed << setprecision(2); // 固定小數兩位
		//cout << " Reading " << i + 1 << ":\n";
		//cout << " Temperature: " << data.temperature << " °C\n";
		//cout << " Humidity: " << data.humidity << " %\n";
		//cout << " Pressure: " << data.pressure << " hPa\n";

		this_thread::sleep_for(chrono::seconds(1));
	}
}

void lesson1() {
	cout << "[cpp-core][lesson1] sensor demo started!\n";

	for (int i = 0; i < 5; ++i)
	{
		double temperature = read_fake_temperature();
		cout << " - temperature = " << temperature << " °C" << endl;
		this_thread::sleep_for(chrono::seconds(1));
	}

}

// 使用 data logger & vector
void lesson3() {
	cout << "[cpp-core][lesson3] Data logger started!\n";

	vector<SensorData> records; // 儲存多筆資料

	// step 1: ofstream 開啟檔案寫入
	// ios::app 表示追加模式，指從檔案尾端繼續寫入，不清除舊資料
	// ofstream file("output.txt" [模式]);
	ofstream logfile("sensor_log.json", ios::app); // 追加模式寫入檔案

	for (int i = 0; i < 5; ++i) {
		SensorData data = read_all_sensors();

		// step 2: 將資料存進 vector
		// 加入 vector
		records.push_back(data);

		//寫入 log 檔案
		string json_str = to_json(data);
		logfile << json_str << "\n";

		cout << json_str << endl;

		// 模擬每秒更新一次 非同步的基礎
		this_thread::sleep_for(chrono::seconds(1));
	}

	logfile.close();
	cout << "\n[cpp-core][lesson3] Logging complete. Total records: "
		<< records.size() << endl;
	cout << "Log saved to sensor_log.json\n";
}

// 使用 map
void lesson4() {
	cout << "[cpp-core][lesson4] Data logger with thresholds started!\n";

	// step 1: 上下限閥值 (可視為簡易 「設定檔」)
	map<string, double> upper_limit{
		{"temperature", 40.0},
		{"humidity",	65.0},
		{"pressure",	101.5}
	};
	map<string, double> lower_limit{
		{"temperature", 36.0},
		{"humidity",	40.0},
		{"pressure",	100.0}
	};

	vector<SensorData> records;
	ofstream logfile("sensor_log.json", ios::app); // 追加模式

	for (int i = 0; i < 8; ++i) {
		SensorData data = read_all_sensors();

		// step 3: 閥值檢查
		check_thresholds(data, upper_limit, lower_limit);

		// step 4: 將資料存進 vector
		// 暫存於記憶體
		records.push_back(data);

		//寫入 log 檔案
		string json_str = to_json(data);
		logfile << json_str << "\n";

		// 同步輸出到主控台
		cout << (data.alert ? "[ALERT]	" : "	")
			<< json_str << endl;

		// 模擬每0.8秒更新一次 非同步的基礎
		this_thread::sleep_for(chrono::milliseconds(800));
	}
}
/*
lesson 5 模擬多感測器
多感測器非同步資料收集
架構: 主線程 (main)
 ├── 溫度感測器 thread
 ├── 濕度感測器 thread
 └── 壓力感測器 thread
 每個感測器執行緒各自回傳一筆數據，再由主線程彙整成一個封包（JSON）並寫入
*/
//mutex data_mutex; // 確保多執行緒 vector 時不衝突
//vector<SensorData_lesson5> buffer;

// 全域序號
atomic<uint64_t> g_seq{ 0 };

int random_ms(int lo, int hi) {
	static random_device rd;
	static mt19937 gen(rd());
	uniform_int_distribution<> dis(lo, hi);
	return dis(gen);
}


// 感測器函式 (溫度 / 濕度 / 壓力共用)
void sensor_task(const string& sensor_name, double min, double max, int count, Logger& logger) {
	// step 1: 準備 thread id 字串
	ostringstream tidss;
	tidss << this_thread::get_id();
	string tid = tidss.str();

	int local = 0;
	for (int i = 0; i < count; ++i) {
		SensorDataPacket data;
		data.sensor = sensor_name;
		data.value = random_value(min, max);
		data.timestamp = get_currentTime();
		/*ostringstream tidss;
		tidss << this_thread::get_id();*/
		//data.thread_id = tidss.str();
		data.thread_id = tid;
		data.global_seq = g_seq.fetch_add(1, memory_order_relaxed);
		data.local_seq = local++;



		// step 2: 
		logger.set_current_sensor_color(sensor_name);
		logger.log_json(data); // 寫檔 + 主控台

		// 
		//this_thread::sleep_for(chrono::milliseconds(600));
		// 改成 隨機 sleep 讓輸出更亂序
		this_thread::sleep_for(chrono::milliseconds(random_ms(150, 700)));
	}
}

/*
* OOP 
* lesson 5 建立多個執行緒
*/
void lesson5() {
	cout << "[cpp-core][lesson6] 10-thread sesor demo started!\n";

	// true = 上鎖; false = 不上鎖 (容易產生 race condition)
	bool USE_LOCK = true;
	Logger logger("multi_sensor_log.jsonl", USE_LOCK);
	
	// 建立 10 個執行緒 (混合不同感測器型別)
	vector<thread> workers;
	workers.reserve(10);

	// 等待全部執行緒完成
	workers.emplace_back(sensor_task, "temperature-1", 36.0, 42.0, 10, ref(logger));
	workers.emplace_back(sensor_task, "temperature-2", 36.0, 42.0, 10, ref(logger));
	workers.emplace_back(sensor_task, "humidity-3", 40.0, 70.0, 10, ref(logger));
	workers.emplace_back(sensor_task, "humidity-4", 40.0, 70.0, 10, ref(logger));
	workers.emplace_back(sensor_task, "pressure-5", 99.0, 102.0, 10, ref(logger));
	workers.emplace_back(sensor_task, "pressure-6", 99.0, 102.0, 10, ref(logger));
	workers.emplace_back(sensor_task, "general-7", 0.0, 100.0, 10, ref(logger));
	workers.emplace_back(sensor_task, "general-8", 0.0, 100.0, 10, ref(logger));
	workers.emplace_back(sensor_task, "general-9", 0.0, 100.0, 10, ref(logger));
	workers.emplace_back(sensor_task, "general-10", 0.0, 100.0, 10, ref(logger));

	for (auto& t : workers) t.join();

	cout << "\nAll threads completed.";
}



int main()
{
	//lesson1();
	//lesson2();
	//lesson3();
	//lesson4();
	lesson5();

	cout << "Press Enter to exit.." << endl;
	cin.get();
	return 0;
}
