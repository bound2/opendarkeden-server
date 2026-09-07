#ifndef __SPECIAL_EVENT_REPOSITORY_H__
#define __SPECIAL_EVENT_REPOSITORY_H__

#include <string>

// The SpecialEvent table: the per-ACCOUNT counter of the special-event
// item hand-out that quest/ActionGiveSpecialEventItem reads and zeroes.
// The table is keyed by Name, but the value passed is the account id,
// not the character name. (Player.SpecialEventCount, which
// SessionRepository reads and saves, is a different column on a
// different table.)
//
// These two statements do not run on the thread's DARKEDEN connection.
// They ask for g_pDatabaseManager->getConnection((int)(long)Thread::self())
// — the int overload, which keys on WorldID, not thread id (the note in
// DatabaseManager.h, translated from Korean: "the main DB server's world
// id is agreed to be 0, and queries to it pass 0"). The gameserver never
// fills m_WorldConnections, so the lookup falls through to
// m_pWorldDefaultConnection, the connection init() opened from the
// WorldDBInfo row with WorldID = 0. In the shipped seeds that row points
// at the same DARKEDEN schema, but it is a separate socket to whatever
// host WorldDBInfo names, and if the row is missing the connection is
// NULL and getConnection Asserts. A process that does not run init()
// (the integration tier) has to hand DatabaseManager a world-default
// connection first (setWorldDefaultConnection).
class SpecialEventRepository {
public:
    virtual ~SpecialEventRepository() {}

    // "SELECT Count FROM SpecialEvent WHERE Name='%s'" — false when the
    // account has no row, the int through getInt otherwise.
    virtual bool loadCount(const std::string& accountID, int& count) = 0;

    // "UPDATE SpecialEvent SET Count = 0 WHERE Name='%s'".
    virtual void resetCount(const std::string& accountID) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSpecialEventRepository.cpp.
SpecialEventRepository& defaultSpecialEventRepository();

#endif
