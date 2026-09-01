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
// this seam's INSERT never sets (it defaults to ''). The guild
// handlers (CGQuitUnion*, SG*Guild*) and the sharedserver's GS*Guild*
// handlers write and drain it with their own inline SQL — their own
// extractions.
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
};

// The process-wide MySQL-backed instance, wired in
// MySQLMessageRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
MessageRepository& defaultMessageRepository();

#endif
