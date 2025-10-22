#include "../include/logger.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <mutex>

// 簡單顏色: 回傳 ANSI 前置碼與重置碼
inline const char* color_for(const std::string& sensor) {
	if (sensor.rfind("temperature", 0) == 0) return "\x1b[31m"; // 紅
	if (sensor.rfind("humidity", 0) == 0) return "\x1b[36m"; // 青
	if (sensor.rfind("pressure", 0) == 0) return "\x1b[33m"; // 黃
	return "\x1b[32m"; // 其他: 綠
}

inline const char* RESET() {
	return "\x1b[0m";
}

namespace {
	// 檔案內全域變數（file-scope static）
	// 可能導致所有 Logger 都排隊等待
	// 只有 Logger.cpp 看的到的靜態所 (若要多 Logger 各自鎖，可把mutex放成成員)
	//std::mutex g_mu;
}

Logger::Logger(const std::string& path, bool use_lock)
	: file_path_(path), use_lock_(use_lock) {
	std::ofstream f(file_path_, std::ios::app);
	if (!f) {
		throw std::runtime_error("Cannot open log file: " + file_path_);
	}
}

void Logger::set_current_sensor_color(const std::string& sensor) {
	std::cout << color_for(sensor);
}

void Logger::log_json(const SensorDataPacket& d, const std::string& tail_note) {
	std::ostringstream line;
	line << std::fixed << std::setprecision(2);
	line << "{"
		<< "\"timestamp\":\""	<< d.timestamp		<< "\","
		<< "\"sensor\":\""		<< d.sensor			<< "\","
		<< "\"value\":"			<< d.value			<< ","
		<< "\"thread_id\":\""	<< d.thread_id		<< "\","
		<< "\"global_seq\":"		<< d.global_seq		<< ","
		<< "\"local_seq\":"		<< d.local_seq
		<< "}";

	// 用 unique_lock + defer_lock 讓「是否上鎖」可選
	std::unique_lock<std::mutex> lk(mu_, std::defer_lock);
	if (use_lock_) lk.lock();

	// 先寫檔，再印出（避免交錯）
	{
		std::ofstream f(file_path_, std::ios::app);
		f << line.str() << "\n";
	}
	std::cout << color_for(d.sensor) << line.str() << tail_note << RESET() << "\n";
}