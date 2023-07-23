
/// A proxy for boost::asio::io_context that collects statistics about posted
/// handlers
class instrumented_io_context : public boost::asio::io_context {
   public:
    /// Initializes the global stats struct after calling the base constructor.
    instrumented_io_context()
        : event_stats_(std::make_shared<EventTracker>()) {}

    void post(std::function<void()> handler, const std::string name);

    void post(std::function<void()> handler,
              std::shared_ptr<StatsHandle> handle);

    void dispatch(std::function<void()> handler, const std::string name);

    EventTracker &stats() const { return *event_stats_; }

   private:
    std::shared_ptr<EventTracker> event_stats_;
};