#include "GatewayNode.hpp"
#include "easylogging++.h"

GatewayNode::GatewayNode()
    : avatarService_(std::make_unique<ChatAvatarService>())
    , roomService_(std::make_unique<ChatRoomService>())
    , messageService_(std::make_unique<PersistentMessageService>())
    , udpLibrary_(std::make_unique<UdpLibrary>()) {
    connection_ = udpLibrary_->CreateConnection();
    InitializeServices();
}

GatewayNode::~GatewayNode() {
    // Resources are automatically cleaned up by unique_ptr
}

void GatewayNode::InitializeServices() {
    // Initialize your services here
    avatarService_->Initialize();
    roomService_->Initialize();
    messageService_->Initialize();
}

void GatewayNode::CreateClient(const std::u16string& address) {
    if (clients_.find(address) == clients_.end()) {
        clients_[address] = std::make_unique<GatewayClient>(connection_.get(), this);
    }
}

void GatewayNode::SendTo(const std::u16string& address, const Message& message) {
    if (clients_.find(address) == clients_.end()) {
        CreateClient(address);
    }
    clients_[address]->Send(message);
}
