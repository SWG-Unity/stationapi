#pragma once
#include "ChatAvatarService.hpp"
#include "ChatEnums.hpp"
#include "ChatRoomService.hpp"
#include "GatewayNode.hpp"
#include "Message.hpp"
#include "PersistentMessageService.hpp"
#include "protocol/AddBan.hpp"
#include "protocol/AddFriend.hpp"
#include "protocol/AddIgnore.hpp"
#include "protocol/AddInvite.hpp"
#include "protocol/AddModerator.hpp"
#include "protocol/CreateRoom.hpp"
#include "protocol/DestroyRoom.hpp"
#include "protocol/EnterRoom.hpp"
#include "protocol/FailoverReLoginAvatar.hpp"
#include "protocol/FriendStatus.hpp"
#include "protocol/GetAnyAvatar.hpp"
#include "protocol/GetPersistentHeaders.hpp"
#include "protocol/GetPersistentMessage.hpp"
#include "protocol/GetRoom.hpp"
#include "protocol/GetRoomSummaries.hpp"
#include "protocol/IgnoreStatus.hpp"
#include "protocol/KickAvatar.hpp"
#include "protocol/LeaveRoom.hpp"
#include "protocol/LoginAvatar.hpp"
#include "protocol/LogoutAvatar.hpp"
#include "protocol/RemoveBan.hpp"
#include "protocol/RemoveFriend.hpp"
#include "protocol/RemoveIgnore.hpp"
#include "protocol/RemoveInvite.hpp"
#include "protocol/RemoveModerator.hpp"
#include "protocol/SendInstantMessage.hpp"
#include "protocol/SendPersistentMessage.hpp"
#include "protocol/SendRoomMessage.hpp"
#include "protocol/SetApiVersion.hpp"
#include "protocol/SetAvatarAttributes.hpp"
#include "protocol/UpdatePersistentMessage.hpp"
#include "protocol/UpdatePersistentMessages.hpp"

#include <mysql/mysql.h>
#include "easylogging++.h"

class GatewayClient : public NodeClient {
public:
    GatewayClient(UdpConnection* connection, GatewayNode* node);
    ~GatewayClient();
    void OnIncoming(std::istringstream& istream);

private:
    GatewayNode* node_;
    ChatAvatarService* avatarService_;
    ChatRoomService* roomService_;
    PersistentMessageService* messageService_;
    MYSQL* conn_;

    template <typename T>
    void HandleIncomingMessage(std::istringstream& istream);

    void SendFriendLoginUpdate(const ChatAvatar* srcAvatar, const ChatAvatar* destAvatar);
    void SendFriendLoginUpdates(const ChatAvatar* avatar);
    void SendFriendLogoutUpdates(const ChatAvatar* avatar);
    void SendDestroyRoomUpdate(const ChatAvatar* srcAvatar, uint32_t roomId, std::vector<std::u16string> targets);
    void SendInstantMessageUpdate(const ChatAvatar* srcAvatar, const ChatAvatar* destAvatar, const std::u16string& message, const std::u16string& oob);
    void SendRoomMessageUpdate(const ChatAvatar* srcAvatar, const ChatRoom* room, uint32_t messageId, const std::u16string& message, const std::u16string& oob);
    void SendEnterRoomUpdate(const ChatAvatar* srcAvatar, const ChatRoom* room);
    void SendLeaveRoomUpdate(const std::vector<std::u16string>& addresses, uint32_t srcAvatarId, uint32_t roomId);
    void SendPersistentMessageUpdate(const ChatAvatar* destAvatar, const PersistentHeader& header);
    void SendKickAvatarUpdate(const std::vector<std::u16string>& addresses, const ChatAvatar* srcAvatar, const ChatAvatar* destAvatar, const ChatRoom* room);
};
