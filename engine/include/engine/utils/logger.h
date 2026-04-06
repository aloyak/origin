#pragma once

#include <string>
#include <functional>

#include <format> // C++20 formatting library, may cause issues, remove if so

class Logger {
public:
    using Callback = std::function<void(const std::string&)>;

    static void setVerbose(int verbose);
    static void setCallback(Callback callback);
    
    static void log(const std::string& message); // Same as info
    
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

    static void clear();
};