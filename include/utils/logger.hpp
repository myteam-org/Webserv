#pragma once

#include <string>
#include <sstream>
#include <fstream>

#define SET_LOG_LEVEL(level) ::Logger::instance().setLevel(level)
#define SET_LOG_FILE(file) ::Logger::instance().setLogFile(file)
#define SET_LOG_OUTPUT(output) ::Logger::instance().setOutput(output)

#define LOG_DEBUG(msg) ::Logger::instance().log(::Logger::kDebug, msg)
#define LOG_INFO(msg) ::Logger::instance().log(::Logger::kInfo, msg)
#define LOG_WARN(msg) ::Logger::instance().log(::Logger::kWarn, msg)
#define LOG_ERROR(msg) ::Logger::instance().log(::Logger::kError, msg)

class Logger {
public:
    enum LogLevel { kDebug, kInfo, kWarn, kError };
    
    enum LogOutput {
        kConsole,
        kFile
    };

    static Logger &instance();

    void setLevel(LogLevel level);
    void setOutput(LogOutput output);
    void setLogFile(const std::string& file);
    void log(LogLevel level, const std::string &message) const;

private:
	static const std::size_t kTimestampBufferSize = 32;
    enum LogColor {
        kColorReset = 0,
        kColorRed = 31,
        kColorGreen = 32,
        kColorYellow = 33,
        kColorBlue = 34,
        kColorGray = 37
    };

    LogLevel logLevel_;
    LogOutput logOutput_;
    mutable std::ofstream logFile_;
    std::string logFileName_;

    Logger();
    ~Logger();
	Logger(const Logger&);
    Logger& operator=(const Logger&);
    
    void openLogFile();
    void closeLogFile();
    static std::string levelToString(LogLevel level);
    static std::string levelToStringNoColor(LogLevel level);
    static std::string getTimestamp();
    static std::string colorize(const std::string &text, LogColor color);
};

// Logger::instance()を通じてどこからでもログ出力できるようにする
// ログレベル(DEBUG,INFOなど)や出力先を指定可能
// 主な機能
// Logger::instance():シングルトンインスタンス取得
// log(level, message):ログ出力（レベルとメッセージ指定）
// setLevel(...):出力する最小ログレベルを設定
// setOutput(...):出力先（コンソールorファイル）を設定
// setLogFile(...):ファイル名を指定（kFile出力時）
// LOG_INFO("msg"):マクロでかんたんにログ出力できる
// #include "logger.hpp" 
// int main() { 
//     // ログレベルをINFOに設定
//     SET_LOG_LEVEL(Logger::kInfo); 
//     // コンソールに出力（デフォルト） 
//     LOG_INFO("Application started"); 
//     // ファイル出力に変更 
//     SET_LOG_FILE("myapp.log"); 
//     SET_LOG_OUTPUT(Logger::kFile); 
//     LOG_INFO("Logging to file only"); 
//     // コンソール出力に戻す 
//     SET_LOG_OUTPUT(Logger::kConsole); 
//     LOG_ERROR("Console only error"); 
//     return 0; 
// }
