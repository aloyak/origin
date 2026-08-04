#pragma once

// multi-thredaded disapacher that allows to pass tasks
// back to the main thread (opengl only allows functions from the main thread)

#include <vector>
#include <functional>
#include <mutex>

class MainThreadDispatcher {
public:
    static MainThreadDispatcher& instance() {
        static MainThreadDispatcher instance;
        return instance;
    }

    void dispatch(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push_back(std::move(task));
    }

    void execute() {
        std::vector<std::function<void()>> currentTasks;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            currentTasks = std::move(m_tasks);
        }

        for (auto& task : currentTasks) {
            task();
        }
    }

private:
    std::vector<std::function<void()>> m_tasks;
    std::mutex m_mutex;
};