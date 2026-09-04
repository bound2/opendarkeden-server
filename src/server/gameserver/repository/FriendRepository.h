#ifndef __FRIEND_REPOSITORY_H__
#define __FRIEND_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the friend-list feature's two tables (task 3.2):
// FriendList, the mutual roster with a per-entry blacklist flag, and
// FriendHistory, the offline message spool. Both are keyed by character
// NAME, not by account.
//
// ================== READ THIS BEFORE USING THIS SEAM ==================
//
// NEITHER TABLE EXISTS. initdb/DARKEDEN.sql defines 374 tables and
// neither FriendList nor FriendHistory is among them, in this schema or
// in USERINFO.sql. No other code in the tree references either table:
// these statements, which came from GCFriendChattingHandler, are the
// only ones. So every method here raises
// against the shipped schema, and it always has.
//
// That is not a defect this round introduced or is fixing — task 3.2
// moves statements without changing what they do — but it decides what
// the seam can honestly promise. It promises the statements are the
// original bytes. It cannot promise they work, and the integration tier
// pins the failure rather than a success it cannot reach.
//
// The consequence is worth stating where a reader will meet it:
// GCFriendChatting is dispatched server-side from
// GamePlayer::processCommand, whose catch (...) turns anything into a
// DisconnectException. END_DB converts the driver's SQLQueryException to
// a const char*, which nothing between here and there catches. So a
// client that opens its friend list is disconnected.
//
// Whoever adds the tables should expect the integration tier's
// FriendMySQL cases to start failing, and should replace them with the
// success-path assertions they were always meant to be. Note the limit
// of that tripwire: it fires only if the columns added match the names
// these statements guess at. A table with any name or type mismatch
// still raises, every assertion stays green, and the tripwire silently
// stops working. Note the tier
// covers six of these nine methods, not all of them: insertBlacklisted,
// hasBlacklisted and deleteMessages have no case, so adding the tables
// gives no signal from those three.
//
// Nothing else in the tree touches either table, so there is no
// "not enclosed" list to keep — which is itself only true because the
// tables do not exist for anything else to touch.
// =====================================================================

// A HAZARD IN THIS INTERFACE, stated because nothing catches it. The
// two insert methods take (friendName, ownerName); every other method
// takes the owner first. That mirrors the statements — the INSERT names
// (Friend_Name, Owner_Name) while every WHERE names Owner_Name first —
// but it means the insert pair and the delete pair are NOT parameter
// compatible, and every parameter here is a const std::string&, so a
// transposition compiles silently and writes or deletes the wrong
// direction.

// One FriendList row as CG_UPDATE reads it: the friend's name and the
// blacklist flag, through getString and getBYTE.
struct FriendListRow {
    std::string friendName;
    BYTE isBlack;
};

// One FriendHistory row as CG_UPDATE reads it, in the statement's own
// column order: the message first, then whose message it is.
struct FriendMessageRow {
    std::string message;
    std::string friendName;
};

class FriendRepository {
public:
    virtual ~FriendRepository() {}

    // CG_ADD_FRIEND_AGREE inserts one row per direction, so the roster is
    // mutual by construction rather than by query. CG_ADD_FRIEND_BLACK
    // inserts a single row with IsBlack = 1, which is a different
    // statement, not the same one with a flag.
    virtual void insertFriend(const std::string& friendName, const std::string& ownerName) = 0;
    virtual void insertBlacklisted(const std::string& friendName, const std::string& ownerName) = 0;

    // The two probes the add-friend request makes. Both select columns
    // their caller never reads — it only asks whether a row came back —
    // so both are kept as bool and the projections stay as written.
    virtual bool friendExists(const std::string& ownerName, const std::string& friendName) = 0;
    // Note the asymmetry: this one is asked with the OTHER character as
    // owner, because it answers "has the person I am adding blacklisted
    // ME?". The IsBlack = 1 test is in the statement, not the caller.
    virtual bool hasBlacklisted(const std::string& ownerName, const std::string& friendName) = 0;

    virtual std::vector<FriendListRow> loadFriends(const std::string& ownerName) = 0;
    // CG_FRIEND_DELETE removes one direction; the caller runs it twice.
    virtual void deleteFriend(const std::string& ownerName, const std::string& friendName) = 0;

    // The offline spool: one row per message, drained and cleared when
    // the recipient next asks for an update.
    virtual void insertMessage(const std::string& message, const std::string& ownerName,
                               const std::string& friendName) = 0;
    virtual std::vector<FriendMessageRow> loadMessages(const std::string& ownerName) = 0;
    virtual void deleteMessages(const std::string& ownerName) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLFriendRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
FriendRepository& defaultFriendRepository();

#endif
