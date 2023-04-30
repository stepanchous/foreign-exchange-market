#include "logger.h"

#include <ctime>
#include <iomanip>

Logger::Logger(std::ostream& log_output) : log_output_(log_output) {}

void Logger::Log(LogType log_type, const std::string& message) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    log_output_ << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    switch (log_type) {
        case LogType::INFO:
            log_output_ << " | INFO | ";
            break;
        case LogType::WARNING:
            log_output_ << " | WARNING | ";
            break;
        case LogType::ERROR:
            log_output_ << "| ERROR | ";
            break;
    }
    log_output_ << message << ";" << std::endl;
}
