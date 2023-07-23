namespace ray {
namespace raylet {
WorkerPool::WorkerPool(instrumented_io_context &io_service, 
                        const NodeID node_id,
                        const std::string node_address,
                        const std::function<int64_t()> &get_num_cpus_available,
                        int num_prestarted_python_workers,
                        int maximum_startup_concurrency,
                        int min_worker_port,
                        int max_worker_port
                        const std::vector<int> &worker_ports)
}
}  // namespace ray