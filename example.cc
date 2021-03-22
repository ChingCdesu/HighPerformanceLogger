//
// example.cc
// HighPerformanceLogger
// 
// Created by _ChingC on 2021/03/22.
// GitHub: https://github.com/ChingCdesu
// Copyright © 2021 _ChingC. All right reserved.
//

#include "logger.h"

int main() {
	// Simply print the log using the default Logger.
	// NOTICE: The Default Logger will only print on console, and the LogLevel is Debug.
	// USAGE: HLOG_DEBUG("MESSAGE");
	HLOG_DEBUG("test_log");
	HLOG_INFO("test_log");
	HLOG_WARN("test_log");
	HLOG_ERROR("test_log");
	
	Logger::LogConfig config;
	// Keep in mind the name of your Logger, which is the key to using your custom Logger.
	config.logger_name = "error-file";
	// Your log file path, support absolute and relative paths.
	config.log_file_name = "error.log";
	// If LogLevel is Error or upper (including Error), message will always be logged.
	// Otherwise, LogLevel of the message from macro calls is lower than the Logger setted will NOT be logged.
	config.log_level = Logger::LogLevel::kError;
	// Do not print on console.
	config.print_console = false;

	// Add your custom logger to logger pool.
	LoggerManager::appendLogger(config);

	// Now you can export log with your custom logger.
	// USAGE: HLOG_ERROR_LOGGER("MESSAGE", "YOUR CURTOM LOGGER NAME");
	HLOG_ERROR_LOGGER("test error", "error-file");

	// Waiting for Logger to write the file.
	std::this_thread::sleep_for(std::chrono::seconds(3));
}