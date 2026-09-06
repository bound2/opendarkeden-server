#ifndef __SPECIAL_EVENT_REPOSITORY_H__
#define __SPECIAL_EVENT_REPOSITORY_H__

#include <string>

// Persistence seam for SpecialEvent (task 3.2, the quest round): the
// per-ACCOUNT counter of the special-event item hand-out that
// quest/ActionGiveSpecialEventItem reads and zeroes. The table is keyed
// by Name, but the value the action passes is pPlayer->getID() — the
// account id, not the character name — so that is what the parameter
// means here. (Player.SpecialEventCount, the column SessionRepository
// reads and saves, is a different thing on a different table.)
//
// WORTH KNOWING: these two statements do not run on the thread's
// DARKEDEN connection. The action asks for
// g_pDatabaseManager->getConnection((int)(long)Thread::self()) — the
// int overload, not the string one every other gameserver statement
// uses. That overload keys on WorldID, not thread id (DatabaseManager.h
// says so in its note — in Korean; translated, "the main DB server's
// world id is agreed to be 0, and queries to it pass 0"); the gameserver
// never fills m_WorldConnections, so the
// lookup falls through to m_pWorldDefaultConnection — the connection
// init() opened from the WorldDBInfo row with WorldID = 0. In the
// shipped seeds that row points at the same DARKEDEN schema, but it is
// a separate socket to whatever host WorldDBInfo names, and if that
// row is missing the connection is NULL and getConnection Asserts. The
// seam keeps the call exactly as written, so it keeps that routing; a
// process that does not run init() (the integration tier) has to hand
// DatabaseManager a world-default connection first
// (setWorldDefaultConnection). This is the only gameserver site that
// used the int overload; CGSayHandler and src/server/PaySystem.cpp
// carry commented-out copies.
//
// Not enclosed: nothing. No other SQL in the tree names SpecialEvent.
class SpecialEventRepository {
public:
    virtual ~SpecialEventRepository() {}

    // "SELECT Count FROM SpecialEvent WHERE Name='%s'" — false when the
    // account has no row (the action tested getRowCount() == 0 and tells
    // the player they did not join), the int through getInt otherwise.
    virtual bool loadCount(const std::string& accountID, int& count) = 0;

    // "UPDATE SpecialEvent SET Count = 0 WHERE Name='%s'".
    virtual void resetCount(const std::string& accountID) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSpecialEventRepository.cpp. An accessor function rather than a
// g_p* extern: ratchet R1 counts those.
SpecialEventRepository& defaultSpecialEventRepository();

#endif
