#include "ChatRoomService.hpp"
#include "ChatAvatarService.hpp"
#include "ChatRoom.hpp"
#include <stdexcept>

ChatRoomService::ChatRoomService(ChatAvatarService* avatarService, sql::Connection* db)
    : avatarService_(avatarService), db_(db) {
    // Initialization logic if needed
}

ChatRoomService::~ChatRoomService() {
    // Cleanup logic if needed
}

void ChatRoomService::LoadRoomsFromStorage(const std::u16string& baseAddress) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(db_->prepareStatement("SELECT * FROM rooms WHERE base_address = ?"));
        pstmt->setString(1, std::string(baseAddress.begin(), baseAddress.end()));
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            auto room = std::make_unique<ChatRoom>(
                res->getUInt("id"),
                std::u16string(res->getString("name").begin(), res->getString("name").end()),
                std::u16string(res->getString("topic").begin(), res->getString("topic").end()),
                std::u16string(res->getString("password").begin(), res->getString("password").end()),
                res->getUInt("attributes"),
                res->getUInt("max_size"),
                std::u16string(res->getString("address").begin(), res->getString("address").end()),
                std::u16string(res->getString("src_address").begin(), res->getString("src_address").end())
            );

            rooms_[room->GetRoomAddress()] = std::move(room);
        }
    } catch (sql::SQLException& e) {
        // Handle database errors
    }
}

ChatRoom* ChatRoomService::CreateRoom(const ChatAvatar* creator, const std::u16string& roomName,
                                      const std::u16string& roomTopic, const std::u16string& roomPassword,
                                      uint32_t roomAttributes, uint32_t maxRoomSize,
                                      const std::u16string& roomAddress, const std::u16string& srcAddress) {
    if (RoomExists(roomAddress)) {
        throw std::runtime_error("Room already exists");
    }

    auto room = std::make_unique<ChatRoom>(nextRoomId_++, roomName, roomTopic, roomPassword,
                                           roomAttributes, maxRoomSize, roomAddress, srcAddress);
    ChatRoom* roomPtr = room.get();
    rooms_[roomAddress] = std::move(room);

    // Persist the new room
    PersistNewRoom(*roomPtr);

    return roomPtr;
}

void ChatRoomService::DestroyRoom(ChatRoom* room) {
    if (room) {
        rooms_.erase(room->GetRoomAddress());

        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(db_->prepareStatement("DELETE FROM rooms WHERE id = ?"));
            pstmt->setUInt(1, room->GetRoomId());
            pstmt->executeUpdate();
        } catch (sql::SQLException& e) {
            // Handle database errors
        }
    }
}

ChatResultCode ChatRoomService::PersistNewRoom(ChatRoom& room) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(db_->prepareStatement(
            "INSERT INTO rooms (id, name, topic, password, attributes, max_size, address, src_address) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
        ));
        pstmt->setUInt(1, room.GetRoomId());
        pstmt->setString(2, std::string(room.GetRoomName().begin(), room.GetRoomName().end()));
        pstmt->setString(3, std::string(room.GetRoomTopic().begin(), room.GetRoomTopic().end()));
        pstmt->setString(4, std::string(room.GetRoomPassword().begin(), room.GetRoomPassword().end()));
        pstmt->setUInt(5, room.GetRoomAttributes());
        pstmt->setUInt(6, room.GetMaxRoomSize());
        pstmt->setString(7, std::string(room.GetRoomAddress().begin(), room.GetRoomAddress().end()));
        pstmt->setString(8, std::string(room.GetSrcAddress().begin(), room.GetSrcAddress().end()));
        pstmt->executeUpdate();
        return ChatResultCode::SUCCESS;
    } catch (sql::SQLException& e) {
        // Handle database errors
        return ChatResultCode::FAILURE;
    }
}

std::vector<ChatRoom*> ChatRoomService::GetRoomSummaries(const std::u16string& startNode,
                                                         const std::u16string& filter) {
    std::vector<ChatRoom*> summaries;
    for (auto& pair : rooms_) {
        if (pair.second->MatchesFilter(filter)) {
            summaries.push_back(pair.second.get());
        }
    }
    return summaries;
}

bool ChatRoomService::RoomExists(const std::u16string& roomAddress) const {
    return rooms_.find(roomAddress) != rooms_.end();
}

ChatRoom* ChatRoomService::GetRoom(const std::u16string& roomAddress) {
    auto it = rooms_.find(roomAddress);
    if (it != rooms_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<ChatRoom*> ChatRoomService::GetJoinedRooms(const ChatAvatar* avatar) {
    std::vector<ChatRoom*> joinedRooms;
    for (auto& pair : rooms_) {
        if (pair.second->IsAvatarInRoom(avatar)) {
            joinedRooms.push_back(pair.second.get());
        }
    }
    return joinedRooms;
}

// Private helper functions for loading and persisting data
void ChatRoomService::DeleteRoom(ChatRoom* room) {
    // Logic to delete the room from persistent storage
}

void ChatRoomService::LoadModerators(ChatRoom* room) {
    // Logic to load moderators from persistent storage
}

void ChatRoomService::PersistModerator(uint32_t moderatorId, uint32_t roomId) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(db_->prepareStatement(
            "INSERT INTO room_moderators (room_id, moderator_id) VALUES (?, ?)"
        ));
        pstmt->setUInt(1, roomId);
        pstmt->setUInt(2, moderatorId);
        pstmt->executeUpdate();
    } catch (sql::SQLException& e) {
        // Handle database errors
    }
}

void ChatRoomService::DeleteModerator(uint32_t moderatorId, uint32_t roomId) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(db_->prepareStatement(
            "DELETE FROM room_moderators WHERE room_id = ? AND moderator_id = ?"
        ));
        pstmt->setUInt(1, roomId);
        pstmt->setUInt(2, moderatorId);
        pstmt->executeUpdate();
    } catch (sql::SQLException& e) {
        // Handle database errors
    }
}

void ChatRoomService::LoadAdministrators(ChatRoom* room) {
    // Logic to load administrators from
