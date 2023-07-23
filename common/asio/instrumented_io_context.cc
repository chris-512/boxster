#include "common/asio/instrumented_io_context.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <utility>

void instrumented_io_context::post(std::function<void()> handler,
                                   const std::string name) {
    if (RayConfig::instance().event_stats()) {
        const auto stats_handle = event_stats_->RecordStart(name);
        handler = [handler = std::move(handler),
                   stats_handle = std::move(stats_handle)]() {
            EventTracker::RecordExecution(handler, std::move(stats_handle));
        };
    }
    auto defer_us = ray::asio::testing::get_delay_us(name);
    if (defer_us == 0) {
        boost::asio::io_context::post(std::move(handler));
    } else {
        RAY_LOG(DEBUG) << "Deferring " << name << " by " << defer_us << "us";
        execute_after(*this, std::move(handler),
                      std::chrono::microseconds(defer_us));
    }
}

void instrumented_io_context::dispatch(std::function<void()> handler,
                                       const std::string name) {
    if (!RayConfig::instance().event_stats()) {
        return boost::asio::io_context::post(std::move(handler));
    }
    const auto stats_handle = event_stats_->RecordStart(name);

    boost::asio::io_context::dispatch(
        [handler = std::move(handler),
         stats_handle = std::move(stats_handle)]() {
            EventTracker::RecordExecution(handler, std::move(stats_handle));
        });
}