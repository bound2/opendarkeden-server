#ifndef __FAKE_SESSION_REPOSITORY_H__
#define __FAKE_SESSION_REPOSITORY_H__

#include <stdexcept>
#include <string>
#include <vector>

#include "repository/SessionRepository.h"

// In-memory SessionRepository for the port check's tests. recordUserIP
// keeps what it was handed; every other method throws, since a test that
// reaches one asked for a statement the decision under test must not make.
class FakeSessionRepository : public SessionRepository {
public:
    struct UserIP {
        std::string name;
        DWORD ip;
        uint port;
        int serverID;
    };
    std::vector<UserIP> userIPs;

    void recordUserIP(const std::string& name, DWORD ip, uint port, int serverID) override {
        userIPs.push_back(UserIP{name, ip, port, serverID});
    }

    void markGuildMemberLoggedOff(const std::string&) override {
        unexpected("markGuildMemberLoggedOff");
    }
    void markGuildMemberLoggedOn(const std::string&) override {
        unexpected("markGuildMemberLoggedOn");
    }
    bool loadPlayerSession(const std::string&, PlayerSessionRow&) override {
        return unexpected("loadPlayerSession");
    }
    bool markPlayerLoggedOn(const std::string&) override {
        return unexpected("markPlayerLoggedOn");
    }
    bool loadPlayerLocation(const std::string&, int&, std::string&) override {
        return unexpected("loadPlayerLocation");
    }
    void markPlayerLoggedOff(const std::string&) override {
        unexpected("markPlayerLoggedOff");
    }
    std::vector<std::string> loadPlayersInGame(int, int) override {
        unexpected("loadPlayersInGame");
        return {};
    }
    void logOffPlayersOfServer(int, int) override {
        unexpected("logOffPlayersOfServer");
    }
    bool loadSpecialEventCount(const std::string&, DWORD&) override {
        return unexpected("loadSpecialEventCount");
    }
    void saveSpecialEventCount(uint, const std::string&) override {
        unexpected("saveSpecialEventCount");
    }
    void deletePCRoomUser(const std::string&) override {
        unexpected("deletePCRoomUser");
    }
    bool loadPCRoomLottoAmount(const std::string&, const std::string&, uint, uint, int&) override {
        return unexpected("loadPCRoomLottoAmount");
    }
    void updatePCRoomLottoAmount(int, const std::string&, const std::string&, uint, uint) override {
        unexpected("updatePCRoomLottoAmount");
    }
    void insertPCRoomLotto(ObjectID_t, const std::string&, uint, uint, const std::string&, Race_t) override {
        unexpected("insertPCRoomLotto");
    }
    void deleteUserIP(const std::string&) override {
        unexpected("deleteUserIP");
    }
    void deleteUserIPsOfServer(int) override {
        unexpected("deleteUserIPsOfServer");
    }
    bool loadUserIP(const std::string&, DWORD&, DWORD&) override {
        return unexpected("loadUserIP");
    }
    void recordSpeedHack(const std::string&, const std::string&, const std::string&, int, int) override {
        unexpected("recordSpeedHack");
    }
    void insertCrashReport(const std::string&, const std::string&, const std::string&, WORD, const std::string&,
                           const std::string&, const std::string&, const std::string&) override {
        unexpected("insertCrashReport");
    }
    bool loadLastLogoutDate(const std::string&, std::string&) override {
        return unexpected("loadLastLogoutDate");
    }
    bool loadUserServerID(const std::string&, int&) override {
        return unexpected("loadUserServerID");
    }
    int countPlayersOnline() override {
        unexpected("countPlayersOnline");
        return 0;
    }
    void denyAccount(const std::string&) override {
        unexpected("denyAccount");
    }
    void insertBugReport(const std::string&, const std::string&, const std::string&) override {
        unexpected("insertBugReport");
    }
    void insertCrashLog(const std::string&, const std::string&, const std::string&, const std::string&,
                        const std::string&, const std::string&) override {
        unexpected("insertCrashLog");
    }
    bool updateUserStatus(uint, int, int) override {
        return unexpected("updateUserStatus");
    }
    void insertUserStatus(int, int, uint) override {
        unexpected("insertUserStatus");
    }

private:
    static bool unexpected(const char* method) {
        throw std::logic_error(std::string("FakeSessionRepository: ") + method + " was not expected");
    }
};

#endif // __FAKE_SESSION_REPOSITORY_H__
