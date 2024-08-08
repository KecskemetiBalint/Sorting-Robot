#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <mutex>
#include <string>

using namespace std;

// Enum to represent log levels
enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger {
public:
    // Get the single instance of the Logger
    static Logger& getInstance(const string& filename = "Logs/logfile.txt") {
        static Logger instance(filename);
        return instance;
    }

    // Deleted methods to prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Logs a message with a given log level
    void log(LogLevel level, const string& message);

private:
    ofstream logFile; // File stream for the log file
    mutex logMutex;   // Mutex for synchronizing log access

    // Private constructor
    Logger(const string& filename);

    // Destructor: Closes the log file
    ~Logger();

    // Converts log level to a string for output
    string levelToString(LogLevel level);
};

#endif // LOGGER_H
