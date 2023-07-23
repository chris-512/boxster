#include "common/asio/io_service_pool.h"

IOServicePool::IOServicePool(size_t io_service_num)
    : io_service_num_(io_service_num) {}

IOServicePool::~IOServicePool() {}

void IOServicePool::Run() {
    for (size_t i = 0; i < io_service_num_; i++) {
        io_services_.emplace_back(std::make_unique<instrumented_io_context>());
        instrumented_io_context &io_service = *io_services_[i];
        threads_.emplace_back([&io_service] {
            boost::asio::io_service::work work(io_service);
            io_service.run();
        });
    }

    RAY_LOG(INFO) << "IOServicePool is running with " << io_service_num_
                  << " io_service.";
}

void IOServicePool::Stop() {
    for (auto &io_service : io_serviecs_) {
        io_service->stop();
    }

    for (auto &thread : threads_) {
        thread.join();
    }

    RAY_LOG(INFO) << "IOServicePool is stopped.";
}