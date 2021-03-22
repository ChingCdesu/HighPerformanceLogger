//
// logger.h
// HighPerformanceLogger
// 
// Created by _ChingC on 2021/03/22.
// GitHub: https://github.com/ChingCdesu
// Copyright © 2021 _ChingC. All right reserved.
//
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <unordered_map>
#include <cstring>
#include "timer.h"

class logger {
public:
	enum LogLevel {
		kDebug = 0,
		kInfo,
		kWarn,
		kError,
		kFatal
	};

	struct LogConfig {
		LogConfig() {
			this->logger_name = "default";
			this->log_file_name = "";
			this->log_level = LogLevel::kDebug;
			this->print_console = true;
		}

		LogConfig(const LogConfig& config) {
			this->logger_name = config.logger_name;
			this->log_file_name = config.log_file_name;
			this->log_level = config.log_level;
			this->print_console = config.print_console;
		}

		std::string logger_name;
		std::string log_file_name;
		LogLevel log_level;
		bool print_console;
	};

	class SourceFile {
	public:
		template<int N>
		inline SourceFile(const char(&arr)[N]) : data_(arr), size_(N - 1) {
			// std::cout<<data_<<std::endl;
			const char* slash = strrchr(data_, splitCh);  // builtin function
			if (slash) {
				data_ = slash + 1;
				size_ -= static_cast<int>(data_ - arr);
			}
		}

		explicit SourceFile(const char* filename) : data_(filename) {
			const char* slash = strrchr(filename, splitCh);
			if (slash) {
				data_ = slash + 1;
			}
			size_ = static_cast<int>(strlen(data_));
		}

		const char* data_;
		int size_;
	private:
#ifdef _WIN32
		const char splitCh = '\\';
#else
		const char splitCh = '/';
#endif
	};

	void log(SourceFile sf, int line, const char* fn, const std::string& msg, LogLevel level = LogLevel::kDebug) {
		// If LogLevel is Error or upper (including Error), message will always be logged.
		// Otherwise, if the LogLevel from macro calls is lower than the Logger setted, message will NOT be logged.
		if (level < LogLevel::kError && this->config.log_level > level) {
			return;
		}
		std::stringstream ss;
		ss << timeFormat() << " " << this->config.logger_name << " " << levelFormat(level) << " " << msg << " - " << sf.data_;
		ss << "@" << fn << ":" << line;
		ss << std::endl;
		auto str = ss.str();
		if (this->config.print_console) {
			setbuf(stdout, 0); // refresh cout buffer
			std::cout << str;
		}
		buf << str;
	}

	logger() {
		this->config = LogConfig();
	}

	logger(const LogConfig& config) {
		this->config = LogConfig(config);
		if (!this->config.log_file_name.empty()) {
			// write the buffer to file for every 1 sec
			t.StartTimer(1000, [this]() {
				// get content in "buf", and reset it
				auto buffer = buf.str();
				buf.str("");
				buf.clear();
				std::ofstream fs(this->config.log_file_name, std::ios::out | std::ios::app);
				fs << buffer;
				fs.close();
				});
		}
	}

private:
	LogConfig config;
	std::stringstream buf;
	timer t;

private:
	static std::string timeFormat() {
		std::stringstream t_ss;
		auto now = std::chrono::system_clock::now();
		auto t_c = std::chrono::system_clock::to_time_t(now);
		t_ss << std::put_time(std::localtime(&t_c), "%F %T UTC %z");
		return t_ss.str();
	}

	static std::string levelFormat(LogLevel level) {
		const std::unordered_map<LogLevel, std::string> map = {
			{LogLevel::kDebug, "DEBUG"},
			{LogLevel::kInfo,  "INFO"},
			{LogLevel::kWarn,  "WARN"},
			{LogLevel::kError, "ERROR"},
			{LogLevel::kFatal, "FATAL"}
		};
		return map.at(level);
	}
};

class loggers {
public:

	static logger* getLogger(const std::string& log_name) {
		return _instances.at(log_name);
	}

	static bool appendLogger(const logger::LogConfig& config) {
		if (exists(config.logger_name)) return false;
		auto _logger = new logger(config);
		_instances.insert({ config.logger_name, _logger });
		return true;
	}

	static void removeLogger(const std::string& log_name) {
		if (exists(log_name)) {
			delete _instances.at(log_name);
			_instances.erase(_instances.find(log_name));
		}
	}

	static bool exists(const std::string& log_name) {
		bool flag = true;
		try {
			_instances.at(log_name);
		}
		catch (const std::out_of_range&) {
			flag = false;
		}
		return flag;
	}

private:
	loggers() = default;
	static logger default_logger;
	static std::unordered_map<std::string, logger*> _instances;
};

// init default logger
// NOTICE: The Default Logger will only print on console, and the LogLevel is Debug.
logger loggers::default_logger = logger();
std::unordered_map<std::string, logger*> loggers::_instances = {
	{"default", &default_logger}
};


// define macros
#define HLOG_DEBUG_LOGGER(MSG, LOGGER) \
    if (loggers::exists(LOGGER)) loggers::getLogger(LOGGER)->log(__FILE__, __LINE__, __FUNCTION__, MSG)
#define HLOG_INFO_LOGGER(MSG, LOGGER) \
    if (loggers::exists(LOGGER)) loggers::getLogger(LOGGER)->log(__FILE__, __LINE__, __FUNCTION__, MSG, \
    logger::LogLevel::kInfo)
#define HLOG_WARN_LOGGER(MSG, LOGGER) \
    if (loggers::exists(LOGGER)) loggers::getLogger(LOGGER)->log(__FILE__, __LINE__, __FUNCTION__, MSG, \
    logger::LogLevel::kWarn)
#define HLOG_ERROR_LOGGER(MSG, LOGGER) \
    if (loggers::exists(LOGGER)) loggers::getLogger(LOGGER)->log(__FILE__, __LINE__, __FUNCTION__, MSG, \
    logger::LogLevel::kError)
#define HLOG_FATAL_LOGGER(MSG, LOGGER) \
    if (loggers::exists(LOGGER)) loggers::getLogger(LOGGER)->log(__FILE__, __LINE__, __FUNCTION__, MSG, \
    logger::LogLevel::kFatal)

#define HLOG_DEBUG(MSG) HLOG_DEBUG_LOGGER(MSG, "default")
#define HLOG_INFO(MSG) HLOG_INFO_LOGGER(MSG, "default")
#define HLOG_WARN(MSG) HLOG_WARN_LOGGER(MSG, "default")
#define HLOG_ERROR(MSG) HLOG_ERROR_LOGGER(MSG, "default")
#define HLOG_FATAL(MSG) HLOG_FATAL_LOGGER(MSG, "default")

#endif //DROGONWEBAPP_LOGGER_H
