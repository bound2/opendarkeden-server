#ifndef __MESSAGE_REPOSITORY_H__
#define __MESSAGE_REPOSITORY_H__

#include <string>
#include <vector>

// The Messages table: one-shot system messages queued for a character
// who was not online to receive them (guild changes made on the
// sharedserver, a pay-zone eviction). The zone delivers and deletes them
// when the character next enters a zone.
//
// The table is keyless (a Receiver index only) with a Sender column the
// INSERT never sets (it defaults to ''). The sharedserver's GS*Guild*
// handlers write the table with their own INSERT.
//
// The union handlers send the INSERT in three spellings that differ only
// by backticks, keyword case and spacing; insertUnionMessage takes an
// enum selecting which text is sent. All parse to the same statement
// (none of Messages, Receiver or Message is reserved), so MySQL cannot
// tell them apart and no test can catch a swapped enumerator; the
// spelling is visible only in SHOW PROCESSLIST, the logs and the
// driver's error output.

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
// MySQLMessageRepository.cpp.
MessageRepository& defaultMessageRepository();

#endif
