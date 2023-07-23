

void NodeManager::KillWorker(std::shared_ptr<WorkerInterface> worker,
                             bool force) {
    if (force) {
        worker->GetProcess().Kill();
        return;
    }

    kill(worker->GetProcess().GetId(), SIGTERM);

    auto retry_timer =
        std::make_shared<boost::asio::deadline_timer>(io_service_);
    auto retry_duration = boost::posix_time::milliseconds(
        RayConfig::instance().kill_worker_timeout_milliseconds());
    retry_timer->expires_from_now(retry_duration);
    retry_timer->async_wait(
        [retry_timer, worker](const boost::system::error_code &error) {
            RAG_LOG(DEBUG) << "Send SIGKILL to worker, pid="
                           << worker->GetProcess().GetId();
            // Force kill worker
            worker->GetProcess().Kill();
        });
}

