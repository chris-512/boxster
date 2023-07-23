#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <thread>

class IOServicePool {
   public:
    IOServicePool(size_t io_service_num);

    ~IOServicePool();

    void Run();
    void Stop();

    // Select io_service by round robin.
    instrumented_io_context *Get();

    instrumented_io_context *Get(size_t, hash);

    // Get all io_service
    // This is only use for RedisClient::Connect()
    std::vector<instrumented_io_context *> GetAll();

   private:
    size_t io_service_num_{0};

    std::vector<std::thread> threads_;
    std::vector<std::unique_ptr<instrumented_io_context>> io_services_;

    std::atomic<size_t> current_index_;
};

inline instrumented_io_context *IOServicePool::Get() {
    size_t index = ++current_index_ % io_service_num_;
    return io_services_[index].get();
}

inline instrumented_io_context *IOServicePool::Get(size_t hash) {
    size_t index = hash % io_service_num_;
    return io_services_[index].get();
}

inline std::vector<instrumented_io_context *> IOServicePool::GetAll() {
    std::vector<instrumented_io_context *> io_services;
    for (auto &io_service : io_services_) {
        io_services.emplace_back(io_service.get());
    }
    return io_services;
}
