#pragma once

#include <string>

#include <format> // C++20 formatting library, may cause issues, remove if so

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