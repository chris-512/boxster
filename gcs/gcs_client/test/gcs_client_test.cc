
class GcsClientTest : public ::testing::TestWithParam<bool> {
   public:
    GcsClientTest() : no_redis_(GetParam()) {
        if (!no_redis_) {
            TestSetupUtil::StartUpRedisServers(std::vector<int>());
        }
    }

    virtual ~GcsClientTest() {
        if (!no_redis_) {
            TestSetupUtil::ShutDownRedisServers();
        }
    }

   protected:
    void SetUp() override {
        if (!no_redis_) {
            config_.redis_address = "127.0.0.1";
            config_.enable_sharding_conn = false;
            config_.redis_port = TEST_REDIS_SERVER_PORTS.front();
        } else {
            config_.redis_port = 0;
            config_.redis_address = "";
        }

        config_.grpc_server_port = 5397;
        config_.grpc_server_name = "MockedGcsServer";
        config_.grpc_server_thread_num = 1;

        client_io_service_ = std::make_unique<instrumented_io_context>();
        client_io_service_thread_ = std::make_unique<std::thread>([this] {
            std::unique_ptr<boost::asio::io_service::work> work(
                new boost::asio::io_service::work(*client_io_service_));
            client_io_service_->run();
        });

        server_io_service_ = std::make_unique<instrumented_io_context>();
        gcs_server_ =
            std::make_unique<gcs::GcsServer>(config_, *server_io_service_);
        gcs_server_->Start();
        server_io_service_thread_ = std::make_unique<std::thread>([this] {
            std::unique_ptr<boost::asio::io_service::work> work(
                new boost::asio::io_service::work(*server_io_service_));
            server_io_service_->run();
        });
    }
}