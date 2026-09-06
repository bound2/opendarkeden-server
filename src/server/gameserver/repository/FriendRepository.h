#ifndef __FRIEND_REPOSITORY_H__
#define __FRIEND_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The friend-list feature's two tables: FriendList, the mutual roster
// with a per-entry blacklist flag, and FriendHistory, the offline message
// spool. Both are keyed by character NAME, not by account.
//
// ======================= READ THIS BEFORE USING =======================
//
// NEITHER TABLE EXISTS. initdb/DARKEDEN.sql defines neither FriendList
// nor FriendHistory, and nor does USERINFO.sql. These statements (from
// GCFriendChattingHandler) are the only code that references them, so
// every method here raises against the shipped schema.
//
// GCFriendChatting is dispatched server-side from
// GamePlayer::processCommand, whose catch (...) turns anything into a
// DisconnectException. END_DB converts the driver's SQLQueryException to
// a const char*, which nothing between here and there catches. So a
// client that opens its friend list is disconnected.
//
// The integration tier's FriendMySQL cases pin that failure. Whoever adds
// the tables should expect them to start failing and replace them with
// success-path assertions. That tripwire fires only if the columns added
// match the names these statements use; a table with any name or type
// mismatch still raises and every assertion stays green. The tier covers
// six of the nine methods: insertBlacklisted, hasBlacklisted and
// deleteMessages have no case.
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

    // The two probes the add-friend request makes: whether a row exists.
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
// MySQLFriendRepository.cpp.
FriendRepository& defaultFriendRepository();

#endif
