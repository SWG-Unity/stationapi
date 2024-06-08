#include "GatewayClient.hpp"

GatewayClient::GatewayClient(UdpConnection* connection, GatewayNode* node)
    : NodeClient(connection), node_{node}, avatarService_{node->GetAvatarService()}, roomService_{node->GetRoomService()}, messageService_{node->GetMessageService()} {
    connection->SetHandler(this);

    // Initialize MySQL connection
    conn_ = mysql_init(nullptr);
    if (conn_ == nullptr) {
        LOG(ERROR) << "mysql_init() failed";
        return;
    }

    if (mysql_real_connect(conn_, "localhost", "user", "password", "database", 0, nullptr, 0) == nullptr) {
        LOG(ERROR) << "mysql_real_connect() failed: " << mysql_error(conn_);
        mysql_close(conn_);
    }
}

GatewayClient::~GatewayClient() {
    if (conn_ != nullptr) {
        mysql_close(conn_);
    }
}

void GatewayClient::OnIncoming(std::istringstream& istream) {
    ChatRequestType request_type = ::read<ChatRequestType>(istream);

    switch (request_type) {
    case ChatRequestType::LOGINAVATAR:
        HandleIncomingMessage<LoginAvatar>(istream);
        break;
    case ChatRequestType::LOGOUTAVATAR:
        HandleIncomingMessage<LogoutAvatar>(istream);
        break;
    case ChatRequestType::CREATEROOM:
        HandleIncomingMessage<CreateRoom>(istream);
        break;
    case ChatRequestType::DESTROYROOM:
        HandleIncomingMessage<DestroyRoom>(istream);
        break;
    case ChatRequestType::SENDINSTANTMESSAGE:
        HandleIncomingMessage<SendInstantMessage>(istream);
        break;
    case ChatRequestType::SENDROOMMESSAGE:
        HandleIncomingMessage<SendRoomMessage>(istream);
        break;
    case ChatRequestType::ADDFRIEND:
        HandleIncomingMessage<AddFriend>(istream);
        break;
    case ChatRequestType::REMOVEFRIEND:
        HandleIncomingMessage<RemoveFriend>(istream);
        break;
    case ChatRequestType::FRIENDSTATUS:
        HandleIncomingMessage<FriendStatus>(istream);
        break;
    case ChatRequestType::ADDIGNORE:
        HandleIncomingMessage<AddIgnore>(istream);
        break;
    case ChatRequestType::REMOVEIGNORE:
        HandleIncomingMessage<RemoveIgnore>(istream);
        break;
    case ChatRequestType::ENTERROOM:
        HandleIncomingMessage<EnterRoom>(istream);
        break;
    case ChatRequestType::LEAVEROOM:
        HandleIncomingMessage<LeaveRoom>(istream);
        break;
    case ChatRequestType::ADDMODERATOR:
        HandleIncomingMessage<AddModerator>(istream);
        break;
    case ChatRequestType::REMOVEMODERATOR:
        HandleIncomingMessage<RemoveModerator>(istream);
        break;
    case ChatRequestType::ADDBAN:
        HandleIncomingMessage<AddBan>(istream);
        break;
    case ChatRequestType::REMOVEBAN:
        HandleIncomingMessage<RemoveBan>(istream);
        break;
    case ChatRequestType::ADDINVITE:
        HandleIncomingMessage<AddInvite>(istream);
        break;
    case ChatRequestType::REMOVEINVITE:
        HandleIncomingMessage<RemoveInvite>(istream);
        break;
    case ChatRequestType::KICKAVATAR:
        HandleIncomingMessage<KickAvatar>(istream);
        break;
    case ChatRequestType::GETROOM:
        HandleIncomingMessage<GetRoom>(istream);
        break;
    case ChatRequestType::GETROOMSUMMARIES:
        HandleIncomingMessage<GetRoomSummaries>(istream);
        break;
    case ChatRequestType::SENDPERSISTENTMESSAGE:
        HandleIncomingMessage<SendPersistentMessage>(istream);
        break;
    case ChatRequestType::GETPERSISTENTHEADERS:
        HandleIncomingMessage<GetPersistentHeaders>(istream);
        break;
    case ChatRequestType::GETPERSISTENTMESSAGE:
        HandleIncomingMessage<GetPersistentMessage>(istream);
        break;
    case ChatRequestType::UPDATEPERSISTENTMESSAGE:
        HandleIncomingMessage<UpdatePersistentMessage>(istream);
        break;
    case ChatRequestType::UPDATEPERSISTENTMESSAGES:
        HandleIncomingMessage<UpdatePersistentMessages>(istream);
        break;
    case ChatRequestType::IGNORESTATUS:
        HandleIncomingMessage<IgnoreStatus>(istream);
        break;
    case ChatRequestType::FAILOVER_RELOGINAVATAR:
        HandleIncomingMessage<FailoverReLoginAvatar>(istream);
        break;
    case ChatRequestType::SETAPIVERSION:
        HandleIncomingMessage<SetApiVersion>(istream);
        break;
    case ChatRequestType::SETAVATARATTRIBUTES:
        HandleIncomingMessage<SetAvatarAttributes>(istream);
        break;
    case ChatRequestType::GETANYAVATAR:
        HandleIncomingMessage<GetAnyAvatar>(istream);
        break;
    default:
        LOG(INFO) << "Unknown request type received: " << static_cast<uint16_t>(request_type);
        break;
    }
}

template <typename T>
void GatewayClient::HandleIncomingMessage(std::istringstream& istream) {
    T message;
    message.Deserialize(istream);
    // Process message using MySQL queries
    // Example:
    std::string query = "SELECT * FROM avatars WHERE avatar_id = " + std::to_string(message.GetAvatarId());
    if (mysql_query(conn_, query.c_str())) {
        LOG(ERROR) << "MySQL query failed: " << mysql_error(conn_);
    } else {
        MYSQL_RES* res = mysql_store_result(conn_);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                // Process the row data
            }
            mysql_free_result(res);
        } else {
            LOG(ERROR) << "mysql_store_result() failed: " << mysql_error(conn_);
        }
    }
}

void GatewayClient::SendFriendLoginUpdate(const ChatAvatar* srcAvatar, const ChatAvatar* destAvatar) {
    node_->SendTo(srcAvatar->GetAddress(), MFriendLogin{destAvatar, destAvatar->GetAddress(), srcAvatar->GetAvatarId(), destAvatar->GetStatusMessage()});
}

void GatewayClient::SendFriendLoginUpdates(const ChatAvatar* avatar) {
    auto& onlineAvatars = avatarService_->GetOnlineAvatars();
    for (auto onlineAvatar : onlineAvatars) {
        if (onlineAvatar->IsFriend(avatar)) {
            SendFriendLoginUpdate(onlineAvatar, avatar);
        }
    }

    for (auto& contact : avatar->GetFriendList()) {
        if (contact.frnd->IsOnline()) {
            Send(MFriendLogin{contact.frnd, contact.frnd->GetAddress(), avatar->GetAvatarId(), contact.frnd->GetStatusMessage()});
        }
    }
}

void GatewayClient::SendFriendLogoutUpdates(const ChatAvatar* avatar) {
    auto& onlineAvatars = avatarService_->GetOnlineAvatars();
    for (auto onlineAvatar : onlineAvatars) {
        if (onlineAvatar->IsFriend(avatar)) {
            node_->SendTo(onlineAvatar->GetAddress(), MFriendLogout{avatar, avatar->GetAddress(), onlineAvatar->GetAvatarId()});
        }
    }
}

void GatewayClient::SendDestroyRoomUpdate(const ChatAvatar* srcAvatar, uint32_t roomId, std::vector<std::u16string> targets) {
    for (auto& address : targets) {
        node_->SendTo(address, MDestroyRoom{srcAvatar, roomId});
    }
}

void GatewayClient::SendInstantMessageUpdate(const ChatAvatar* srcAvatar, const ChatAvatar* destAvatar, const std::u16string& message, const std::u16string& oob) {
    node_->SendTo(destAvatar->GetAddress(), MInstantMessage{srcAvatar, destAvatar->GetAvatarId(), message, oob});
}

void GatewayClient::SendRoomMessageUpdate(const ChatAvatar* srcAvatar, const ChatRoom* room, uint32_t messageId, const std::u16string& message, const std::u16string& oob) {
    auto connectedAddresses = room->GetConnectedAddresses();
    for (auto& address : connectedAddresses) {
        node_->SendTo(address, MRoomMessage{srcAvatar, room->GetRoomId(), room->GetAvatarIds(srcAvatar), message, oob, messageId});
    }
}

void GatewayClient::SendEnterRoomUpdate(const ChatAvatar* srcAvatar, const ChatRoom* room) {
    for (const auto& address : room->GetConnectedAddresses()) {
        node_->SendTo(address, MEnterRoom{srcAvatar, room->GetRoomId()});
    }
}

void GatewayClient::SendLeaveRoomUpdate(const std::vector<std::u16string>& addresses, uint32_t srcAvatarId, uint32_t roomId) {
    for (const auto& address : addresses) {
        node_->SendTo(address, MLeaveRoom{srcAvatarId, roomId});
    }
}

void GatewayClient::SendPersistentMessageUpdate(const ChatAvatar* destAvatar, const PersistentHeader& header) {
    if (destAvatar) {
        node_->SendTo(destAvatar->GetAddress(), MPersistentMessage{destAvatar->GetAvatarId(), header});
    }
}

void GatewayClient::SendKickAvatarUpdate(const std::vector<std::u16string>& addresses, const ChatAvatar* srcAvatar, const ChatAvatar* destAvatar, const ChatRoom* room) {
    for (const auto& address : addresses) {
        node_->SendTo(address, MKickAvatar{srcAvatar, destAvatar, room->GetRoomName(), room->GetRoomAddress()});
    }
}
