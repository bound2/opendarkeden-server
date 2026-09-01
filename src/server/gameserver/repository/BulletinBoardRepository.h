#ifndef __BULLETIN_BOARD_REPOSITORY_H__
#define __BULLETIN_BOARD_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the BulletinBoardObject table (task 3.2, the
// Zone milestone): the player-written notice "corpses" placed in a
// zone, persisted so they outlive a restart. Rows are keyed by server
// AND zone; the message text is already escaped by the caller
// (Guild::correctString) and the time limit is a datetime text the
// caller formats.
//
// What loadForZone() returns — each field typed to the driver getter
// the inline code called: ID/X/Y/Type through getInt, Message and
// TimeLimit through getString (the caller parses TimeLimit).
struct BulletinBoardRow {
    int id;
    int x;
    int y;
    std::string message;
    int type;
    std::string timeLimit;
};

class BulletinBoardRepository {
public:
    virtual ~BulletinBoardRepository() {}

    // A new notice. Returns the affected-row count — the caller logs a
    // zero (the auto-increment ID is never read back).
    virtual int insert(int serverID, ZoneID_t zoneID, int x, int y, const std::string& message, uint type,
                       const std::string& timeLimit) = 0;

    // Every notice of a zone on this server.
    virtual std::vector<BulletinBoardRow> loadForZone(int serverID, ZoneID_t zoneID) = 0;

    // Drops one notice by ID (an expired one, found at load time).
    virtual void remove(uint id) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLBulletinBoardRepository.cpp. An accessor function rather than a
// g_p* extern: ratchet R1 counts those.
BulletinBoardRepository& defaultBulletinBoardRepository();

#endif
