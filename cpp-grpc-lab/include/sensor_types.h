#pragma once
#include <string>
#include <cstdint>

struct SensorDataPacket {
	std::string sensor;
	double value{ 0.0 };
	std::string timestamp;
	std::string thread_id;

	// °lÂÜ¿é¥X²v
	uint64_t global_seq{ 0 };
	int local_seq{ 0 };
};