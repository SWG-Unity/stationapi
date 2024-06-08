#pragma once

#include "ChatAvatarService.hpp"
#include "ChatRoomService.hpp"
#include "PersistentMessageService.hpp"
#include "UdpLibrary.hpp"
#include "GatewayClient.hpp"
#include "Message.hpp"

#include <unordered_map>
#include <string>
#include <memory>

class GatewayNode {
public:
    GatewayNode();
    ~GatewayNode();

    ChatAvatarService* GetAvatarService() const { return avatarService_.get(); }
    ChatRoomService* GetRoomService() const { return roomService_.get(); }
    PersistentMessageService* GetMessageService() const { return messageService_.get(); }

    void SendTo(const std::u16string& address, const Message& message);

private:
    std::unique_ptr<ChatAvatarService> avatarService_;
    std::unique_ptr<ChatRoomService> roomService_;
    std::unique_ptr<PersistentMessageService> messageService_;
    std::unordered_map<std::u16string, std::unique_ptr<GatewayClient>> clients_;
    std::unique_ptr<UdpConnection> connection_;
    std::unique_ptr<UdpLibrary> udpLibrary_;

    void InitializeServices();
    void CreateClient(const std::u16string& address);
};
