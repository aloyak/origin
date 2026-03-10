#pragma once

#include <string>

// Meant to be used with std::format, which is C++20 so some compilers might not support this
class Logger {
public:
    static void setVerbose(int verbose);
    
    static void log(const std::string& message); // Same as info
    
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

    static void clear();
};