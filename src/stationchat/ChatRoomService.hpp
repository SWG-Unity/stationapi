
#pragma once

#include "ChatEnums.hpp"
#include "ChatRoom.hpp"
#include "ChatAvatarService.hpp"
#include "mysql_driver.h"
#include "mysql_connection.h"
#include "cppconn/prepared_statement.h"

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class ChatRoomService {
public:
    ChatRoomService(ChatAvatarService* avatarService, sql::Connection* db);
    ~ChatRoomService();

    void LoadRoomsFromStorage(const std::u16string& baseAddress);

    ChatRoom* CreateRoom(const ChatAvatar* creator, const std::u16string& roomName,
                         const std::u16string& roomTopic, const std::u16string& roomPassword,
                         uint32_t roomAttributes, uint32_t maxRoomSize,
                         const std::u16string& roomAddress, const std::u16string& srcAddress);

    void DestroyRoom(ChatRoom* room);

    ChatResultCode PersistNewRoom(ChatRoom& room);

    std::vector<ChatRoom*> GetRoomSummaries(const std::u16string& startNode,
                                            const std::u16string& filter = u"");

    bool RoomExists(const std::u16string& roomAddress) const;
    ChatRoom* GetRoom(const std::u16string& roomAddress);

    std::vector<ChatRoom*> GetJoinedRooms(const ChatAvatar* avatar);

private:
    friend class ChatRoom;
    void DeleteRoom(ChatRoom* room);
    void LoadModerators(ChatRoom* room);
    void PersistModerator(uint32_t moderatorId, uint32_t roomId);
    void DeleteModerator(uint32_t moderatorId, uint32_t roomId);
    void LoadAdministrators(ChatRoom* room);
    void PersistAdministrator(uint32_t administratorId, uint32_t roomId);
    void DeleteAdministrator(uint32_t administratorId, uint32_t roomId);
    void LoadBanned(ChatRoom* room);
    void PersistBanned(uint32_t bannedId, uint32_t roomId);
    void DeleteBanned(uint32_t bannedId, uint32_t roomId);

    uint32_t nextRoomId_ = 0;
    std::unordered_map<std::u16string, std::unique_ptr<ChatRoom>> rooms_;
    ChatAvatarService* avatarService_;
    sql::Connection* db_;
};
