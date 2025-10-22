#pragma once
#include <string>
#include "sensor_types.h"
#include <mutex>

class Logger {
public: 
	// explicit 代表「你必須明確地建立一個 Logger 物件」
	// 不能讓編譯器自動幫你把一個 std::string（或 const char*）轉換成 Logger
	explicit Logger(const std::string& path, bool use_lock = true);

	// 設定顏色或附加註記
	void set_current_sensor_color(const std::string& sensor);
	

	void log_json(const SensorDataPacket& d, const std::string& tail_note = "");
private:
	std::string file_path_;
	bool use_lock_{ true };
	std::mutex mu_;
};