#ifndef __MESSAGE_REPOSITORY_H__
#define __MESSAGE_REPOSITORY_H__

#include <string>
#include <vector>

// Persistence seam for the Messages table (task 3.2, the Zone
// milestone): one-shot system messages queued for a character who was
// not online to receive them (guild changes made on the sharedserver,
// a pay-zone eviction). The zone delivers and deletes them when the
// character next enters a zone.
//
// The table is keyless (a Receiver index only) with a Sender column
// this seam's INSERT never sets (it defaults to ''). The five
// guild-union handlers' notices are enclosed here now; what still keeps
// its own inline SQL is the gameserver's CGExpelGuild handler, its
// SGDeleteGuildOK / SGModifyGuildOK / SGModifyGuildMemberOK handlers
// (SELECT and DELETE, draining, never inserting) and the sharedserver's
// GS*Guild* handlers.
//
// A note on the spellings below. The union handlers write the same
// INSERT three different ways, and none of the three is the one
// insertMessage() already carries. Backticks around an identifier and
// the space before a VALUES list are inert to MySQL, so all four are
// the same statement — but task 3.2 moves statements without rewriting
// them, so the spelling is a parameter here rather than something
// normalised away. Collapsing them is a one-line follow-up whenever
// that is wanted.
// Which of the union handlers' three spellings to write.
enum UnionNoticeSpelling {
    // CGQuitUnionHandler, CGQuitUnionAcceptHandler, CGQuitUnionDenyHandler.
    UNION_NOTICE_PLAIN,
    // CGDenyUnionHandler: every identifier backticked, and a space
    // before the VALUES list.
    UNION_NOTICE_QUOTED_SPACED,
    // CGAcceptUnionHandler: backticked, no space.
    UNION_NOTICE_QUOTED,
    UNION_NOTICE_SPELLING_MAX
};

class MessageRepository {
public:
    virtual ~MessageRepository() {}

    // Every queued Message for the receiver, as the driver's getString
    // returned them. No ORDER BY: the order is the optimizer's choice,
    // not a contract.
    virtual std::vector<std::string> loadMessages(const std::string& receiver) = 0;

    // Drops every row of the receiver.
    virtual void deleteMessages(const std::string& receiver) = 0;

    // Queues one message (the pay-zone eviction notice).
    virtual void insertMessage(const std::string& receiver, const std::string& message) = 0;

    // The same, in whichever of the union handlers' three spellings the
    // caller used. Every caller passes a guild master's name and a
    // StringPool line.
    virtual void insertUnionNotice(UnionNoticeSpelling spelling, const std::string& receiver,
                                   const std::string& message) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLMessageRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
MessageRepository& defaultMessageRepository();

#endif
