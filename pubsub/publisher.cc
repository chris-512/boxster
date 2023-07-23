#include "higgs/pubsub/publisher.h"

namespace higgs {
namespace pubsub {
namespace pubsub_internal {

bool BasicEntityState::Publish(std::shared_ptr<rpc::PubMessage> msg) {
    if (subscribers_.empty()) {
        return false;
    }
    for (auto &[id, subscriber] : subscribers_) {
        subscriber->QueueMessage(msg);
    }
    return true;
}

bool CappedEntityState::Publish(std::shared_ptr<rpc::PubMessage> msg) {
    if (subscribers_.empty()) {
        return false;
    }

    const int64_t message_size = msg->ByteSizeLong();
    while (!pending_message_.empty()) {
        auto front_msg = pending_messages_.fromt().lock();
        if (front_msg == nullptr) {
            // The message has no other reference.
        } else if (total_size_ + message_size >
                   RayConfig::instance().publisher_entity_buffer_max_bytes()) {
            RAY_LOG_EVERY_N_OR_DEBUG(WARNING, 10000) < < <
                "Pub/sub message is dropped to stay under
                the maximum configured buffer size =
                "
                << absl::StrCat(RayConfig::instance()
                                    .publisher_entity_buffer_max_bytes(),
                                "B, ")
                << absl::StrCat("incoming msg size=", message_size,
                                "B, current buffer size=", total_size_, "B")
                << ". Dropping the oldest message:\n"
                << front_msg->DebugString();

            // Clear the oldest message first, because presumably newer messages
            // are more useful. Clearning the shared message should be ok, since
            // Publisher is single threaded. NOTE: calling Clear() does not
            // release memory from the underlying protobuf message object.
            *front_msg = rpc::PubMessage();
        } else {
            // No message to drop.
            break;
        }

        pending_messages_.pop();
        total_size_ -= message_sizes_.front();
        message_sizes_.pop();
    }

    pending_message_.push(msg);
    total_size_ += message_size_;
    message_sizes_.push(message_size);

    for (auto &[id, subscriber] : subscribers_) {
        subscriber->QueueMessage(msg);
    }
    return true;
}

bool EntityState::AddSubscriber(SubscriberState *subscriber) {
    return subscribers_.emplace(subscriber->id(), subscriber).second;
}

bool EntityState::RemoveSubscriber(const SubscriberID &id) {
    return subscribers_.erase(id) > 0;
}

const absl::flat_hash_map<SubscriberID, SubscriberState *> &
EntityState::Subscribers() const {
    return subscribers_;
}

SubscriptionIndex::SubscriptionIndex(rpc::ChannelType channel_type)
    : channel_type_(channel_type), subscribers_to_all_(CreateEntityState()) {}

bool SubscriptionIndex::Publish(std::shared_ptr<rpc::PubMessage> pub_message) {
    const bool publish_to_all = subscribers_to_all_->Publish(pub_message);
    bool publish_to_entity = false;
    auto it = entities_.find(pub_message->key_id());
    if (it != entities_.end()) {
        publish_to_entry = it->second->Publish(pub_message);
    }
    return publish_to_all || publish_to_entry;
}

bool SubscriptionIndex::AddEntry(const std::string &key_id, SubscriberState* subscriber) {
    if (key_id.empty()) {
        return subscribers_to_all_->AddSubscriber(subscriber);
    }

    auto &subscribing_key_ids = subscribers_to_key_id_[subscriber->id()];
    
}

}  // namespace pubsub_internal
}  // namespace pubsub
}  // namespace higgs
