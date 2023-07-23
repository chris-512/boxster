#include <deque>
#include <queue>
#include <string>

namespace higgs {
namespace pubsub {

using SubscriberID = UniqueID;
using PublisherID = UniqueID;

namespace pub_internal {

class SubscriberState;

/// State for an entity / topic in a pub/sub channel.
class EntityState {
   public:
    virtual ~EntityState() = default;

    virtual bool Publish(std::shared_ptr<rpc::PubMessage> pub_message) = 0;

    /// Manages the set of subscribers of this entity.
    bool AddSubscriber(SubscriberState *subscriber);
    bool RemoveSubscriber(const SubscriberID &id);

    /// Gets the current set of subscribers, keyed by subscriber IDs.
    const absl::flat_hash_map<SubscriberID, SubscriberState *> &Subscribers()
        const;

   protected:
    // Subscribers of this entity
    // The underlying SubscriberState is owned by Publisher.
    absl::flat_hash_map<SubscriberID, SubscriberState *> subscribers_;
};

class BasicEntityState : public EntityState {
   public:
    bool Publish(std::shared_ptr<rpc::PubMessage> pub_message) override;
};

class CappedEntityState : public EntityState {
   public:
    bool Publish(std::shared_ptr<rpc::PubMessage> pub_message) override;

   private:
    // Tracks inflight messages. The messages have shared ownership by
    // individual subscribers, and get deleted after no subscriber has
    // the message in buffer.
    std::queue<std::weak_ptr<rpc::PubMessage>> pending_messages;
    // Size of each inflight message.
    std::queue<int64_t> message_sizes_;
    // Total size of inflight messages.
    int64_t total_size_ = 0;
};

/// Per-channel two-way index for subscribers and the keys they subscribe to.
/// Also supports subscribers to all keys in the channel.
class SubscriptionIndex {
   public:
    SubscriptionIndex(rpc::ChannelType channel_type);
    ~SubscriptionIndex() = default;

    SubscriptionIndex(SubscriptionIndex &&) noexcept = default;
    SubscriptionIndex &operator=(SubscriptionIndex &&) noexcept = default;

    /// Publishes the message to relevant subscribers.
    /// Returns true if there are subscribers listening on the entity key of the
    /// message, returns false otherwise.
    bool Publish(std::shared_ptr<rpc::PubMessage> pub_message);

    /// Adds a new subscriber and the key it subscribes to.
    /// When `key_id` is empty, the subscriber subscribes to all keys.
    /// NOTE: The method is idempotent. If it adds a duplicated entry, it will
    /// be no-op.
    bool AddEntry(const std::string &key_id, SubscriberState *subscriber);

private:
    rpc::ChannelType channel_type_;
    std::unique_ptr<EntityState> subscribers_to_all_;
    absl::flat_hash_map<std::string, std::unique_ptr<EntityState>> entities;
};

}  // namespace pub_internal

}  // namespace pubsub
}  // namespace higgs