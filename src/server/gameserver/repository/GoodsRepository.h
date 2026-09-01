#ifndef __GOODS_REPOSITORY_H__
#define __GOODS_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// One undelivered web-shop purchase row (GoodsListObject), as the domain
// sees it: plain data, no SQL types.
struct GoodsRecord {
    // GoodsListObject.ID (a bigint), carried as the string the DB returned
    // — the delivery flow only ever hands it back to takeOne() and logs.
    std::string id;
    DWORD goodsID;
    int num;
};

// Persistence seam for the GoodsListObject table (task 3.2): items bought
// on the game's website, waiting to be picked up in-game. Reached through
// the second per-thread connection — see the connection quirk on the
// MySQL implementation.
class GoodsRepository {
public:
    virtual ~GoodsRepository() {}

    // Every not-yet-delivered purchase for a character on one world.
    virtual std::vector<GoodsRecord> loadPending(int world, const std::string& playerID,
                                                 const std::string& characterName) = 0;

    // Deliver one unit of a purchase row: decrement its count and mark it
    // taken once the count runs out. False when no row matched the id.
    // A row already at Num=0 does NOT report false: the decrement of the
    // UNSIGNED column raises ER_DATA_OUT_OF_RANGE, the row is left
    // untouched, and the error escapes as an exception — see the MySQL
    // implementation's quirk notes.
    virtual bool takeOne(const std::string& id) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLGoodsRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
GoodsRepository& defaultGoodsRepository();

#endif
