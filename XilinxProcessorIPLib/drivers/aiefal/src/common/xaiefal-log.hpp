// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <functional>

#pragma once

namespace xaiefal {
	enum class LogLevel {
		DEBUG,
		INFO,
		WARN,
		ERROR,
		MAX,
	};

	class Logger
	{
	private:
		Logger() : Level(LogLevel::INFO) {}
	public:
		LogLevel Level;
		std::ofstream of;
		std::ofstream ofnull;

		~Logger() {}

		void setLogLevel(LogLevel L) {
			Level = L;
		}

		LogLevel getLogLevel() {
			return Level;
		}

		void setLogFile(const std::string& File) {
			of.open(File);
		}

		Logger(const Logger&) = delete;
		Logger& operator=(const Logger &) = delete;
		Logger(Logger &&) = delete;
		Logger & operator=(Logger &&) = delete;
		static Logger &get() {
			static Logger logger;

			return logger;
		}
		static std::ostream &log(LogLevel L=LogLevel::MAX) {
			const char* label;

			switch(L) {
				case LogLevel::DEBUG: label = "XAIEFAL: DEBUG: "; break;
				case LogLevel::INFO:  label = "XAIEFAL: INFO: "; break;
				case LogLevel::WARN:  label = "XAIEFAL: WARN: "; break;
				case LogLevel::ERROR: label = "XAIEFAL: ERROR: "; break;
				default: label = ""; break;
			}

			if (get().Level <= L) {
				if (get().of.rdbuf()->is_open()) {
					get().of << label;
					return get().of;
				} else {
					std::cout << label;
					return std::cout;
				}
			} else {
				return get().ofnull;
			}
		}

	};
}
