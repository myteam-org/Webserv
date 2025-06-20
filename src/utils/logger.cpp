#include "logger.hpp"
#include <iostream>
#include <ctime>

Logger &Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : logLevel_(kDebug), logOutput_(kConsole), logFileName_("app.log") {}

Logger::~Logger() {
    closeLogFile();
}

void Logger::setLevel(const LogLevel level) {
    this->logLevel_ = level;
}

void Logger::setOutput(const LogOutput output) {
    this->logOutput_ = output;
    
    if (output == kFile) {
        if (!logFile_.is_open()) {
            openLogFile();
        }
    } else {
        closeLogFile();
    }
}

void Logger::setLogFile(const std::string& file) {
    if (logFile_.is_open()) {
        closeLogFile();
    }
    
    logFileName_ = file;
    
    if (logOutput_ == kFile) {
        openLogFile();
    }
}

void Logger::openLogFile() {
    logFile_.open(logFileName_.c_str(), std::ios::app);
    
    if (!logFile_.is_open()) {
        std::cerr << "Error opening log file: " << logFileName_ 
                  << ". Defaulting to app.log." << std::endl;
        logFileName_ = "app.log";
        logFile_.open(logFileName_.c_str(), std::ios::app);
    }
    
    if (logFile_.is_open()) {
        const std::string timestamp = getTimestamp();
        logFile_ << "[" << timestamp << "] [INFO] Log file opened: " 
                 << logFileName_ << std::endl;
        logFile_.flush();
    }
}

void Logger::closeLogFile() {
    if (logFile_.is_open()) {
        const std::string timestamp = getTimestamp();
        logFile_ << "[" << timestamp << "] [INFO] Log file closed." << std::endl;
        logFile_.close();
    }
}

void Logger::log(const LogLevel level, const std::string &message) const {
    if (level < this->logLevel_) {
        return;
    }

    const std::string timestamp = getTimestamp();
    
    if (logOutput_ == kConsole) {
        const std::string levelStr = levelToString(level);
        std::ostringstream logEntry;
        logEntry << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
        std::cout << logEntry.str();
    } else if (logOutput_ == kFile && logFile_.is_open()) {
        const std::string levelStr = levelToStringNoColor(level);
        logFile_ << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
        logFile_.flush();
    }
}

std::string Logger::levelToString(const LogLevel level) {
    switch (level) {
        case kDebug:
            return colorize("DEBUG", kColorBlue);
        case kInfo:
            return colorize("INFO", kColorGreen);
        case kWarn:
            return colorize("WARN", kColorYellow);
        case kError:
            return colorize("ERROR", kColorRed);
        default:
            return colorize("UNKNOWN", kColorGray);
    }
}

std::string Logger::levelToStringNoColor(const LogLevel level) {
    switch (level) {
        case kDebug:
            return "DEBUG";
        case kInfo:
            return "INFO";
        case kWarn:
            return "WARN";
        case kError:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

std::string Logger::getTimestamp() {
    const std::time_t nowTime = std::time(NULL);
    const tm *nowLocal = std::localtime(&nowTime);
	char buffer[kTimestampBufferSize];
    std::strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S%z", nowLocal);
    return std::string(buffer);
}

std::string Logger::colorize(const std::string &text, const LogColor color) {
    std::ostringstream oss;
    oss << "\033[" << color << "m" << text << "\033[0m";
    return oss.str();
}

