#include <iostream>
#include <string>

enum class LogType {
    INFO,
    WARNING,
    ERROR,
};

class Logger {
   public:
    Logger(std::ostream& log_output);

    void Log(LogType log_type, const std::string& message);

   private:
    std::ostream& log_output_;
};
