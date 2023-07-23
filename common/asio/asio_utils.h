#pragma once

#include <boost/asio.hpp>
#include <chrono>

template <typename Duration>
std::shared_ptr<boost::asio::deadline_timer> execute_after(
    instrumented_io_context &io_context, std::function<void()> fn,
    Duration delay_duration) {
    auto timer = std::make_shared<boost::asio::deadline_timer>(io_context);
    auto delay = boost::posix_time::microseconds(
        std::chrono::duration_cast<std::chrono::microseconds>(delay_duration)
            .count());
    timer->expires_from_now(delay);
    timer->async_wait(
        [timer, fn = std::move(fn)](const boost::system::error_code &error) {
            if (error != boost::asio::error_operation_aborted && fn) {
                fn();
            }
        });

    return timer;
}