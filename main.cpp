#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <sstream>
#include <chrono>

class Logger {
private:
    std::ofstream logFile;
    std::mutex logMutex;

    Logger() {
        logFile.open("log.txt", std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file.");
        }
    }

public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        logFile << message << std::endl;
        std::cout << "Logged: " << message << std::endl;
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
};

template <typename Func, typename... Args>
void runInThreadPool(int numThreads, Func&& func, Args&&... args) {
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(std::forward<Func>(func), std::forward<Args>(args)...);
    }
    for (auto& t : threads) {
        t.join();
    }
}

template <int N>
struct Fibonacci {
    static constexpr int value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};

template <>
struct Fibonacci<0> {
    static constexpr int value = 0;
};

template <>
struct Fibonacci<1> {
    static constexpr int value = 1;
};

void logTask(int id) {
    for (int i = 0; i < 5; ++i) {
        std::ostringstream oss;
        oss << "Thread " << id << " - Fibonacci(10) = " << Fibonacci<10>::value;
        Logger::getInstance().log(oss.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    try {
        constexpr int numThreads = 3;
        runInThreadPool(numThreads, logTask, 1);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
