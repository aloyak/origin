#pragma once

#include <string>
#include <functional>
#include <format> // C++20

class Logger {
public:
    using Callback = std::function<void(const std::string&)>;

    static void setVerbose(int verbose);
    static void setCallback(Callback callback);

    static void log(const std::string& message);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

    template<typename... Args>
    static void log(std::format_string<Args...> fmt, Args&&... args) {
        log(std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void debug(std::format_string<Args...> fmt, Args&&... args) {
        debug(std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void info(std::format_string<Args...> fmt, Args&&... args) {
        info(std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void warn(std::format_string<Args...> fmt, Args&&... args) {
        warn(std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void error(std::format_string<Args...> fmt, Args&&... args) {
        error(std::format(fmt, std::forward<Args>(args)...));
    }

    static void clear();
};