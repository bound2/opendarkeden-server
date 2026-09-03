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
// insertMessage() already carries: they differ by backticks, by the
// space before the VALUES list, and (against insertMessage) by the
// spaces inside the column list and by VALUES against values.
//
// For THESE identifiers all four parse to the same statement — none of
// Messages, Receiver or Message is reserved, so the backticks are
// redundant, and MySQL's tokenizer does not care about the spacing or
// the keyword case. That is narrower than "inert": backticks are load
// bearing elsewhere in this tree (see MySQLGuildRepository.cpp on
// `Rank`, reserved on MySQL 8), the spelling reaches SHOW PROCESSLIST,
// the general and slow logs and the driver's own error output
// verbatim, MySQL 5.7's query cache keys on the exact bytes, and the
// longer spelling leaves nine fewer bytes of the 2048 that
// Statement::executeQuery will format into.
//
// Task 3.2 moves statements without rewriting them, so the spelling is
// a parameter here rather than something normalised away. Collapsing
// them later is a small change — the parameter, six call sites across
// five files, and a test — whenever that is wanted. Note what keeping
// them costs: because the variants are
// indistinguishable to MySQL, no test can tell a swapped enumerator
// from a correct one. The mapping from call site to spelling is held
// by review, not by the suite.

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
