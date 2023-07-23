
namespace ray {
namespace raylet {
/// A constructor responsible for initializing the state of a worker.
Worker::Worker(const JobID &job_id, const int runtime_env_hash,
               const WorkerID &worker_id, const Language &language,
               rpc::WorkerType worker_type, const std::string &ip_address,
               std::shared_ptr<ClientConnection> connection,
               rpc::ClientCallManager &client_call_manager,
               StartupToken startup_token)
    : worker_id_(worker_id),
      startup_token_(startup_token),
      language_(language),
      worker_type_(worker_type),
      ip_address_(ip_address),
      assigned_port_(-1),
      port_(-1),
      connection_(connection),
{}

void Worker::MarkDead() { dead_ = true; }



}  // namespace raylet
}  // namespace ray