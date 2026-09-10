#ifndef __SMS_MESSAGE_REPOSITORY_H__
#define __SMS_MESSAGE_REPOSITORY_H__

#include <string>

#include "Types.h"

// The SMS relay tables SMSServiceThread writes: one row per outgoing
// text (uds_msg) plus a row naming it in the relay's send queue
// (msg_queue).
//
// Connection: not one of DatabaseManager's. The relay is a MySQL server
// of its own, named by the gameserver configuration's SMS_DB_HOST,
// SMS_DB_DB, SMS_DB_USER, SMS_DB_PASSWORD and SMS_DB_PORT keys; this
// repository opens that connection and owns it for the life of the
// process. Nothing here touches the game schema.
//
// Neither table is in initdb/, so the MySQL integration tier cannot pin
// this repository, and on the shipped schema every statement below
// fails. Nothing runs them either: GameServer::start() does not start
// SMSServiceThread, so the loop that would call insertMessage and
// enqueue never turns. The thread's queue is still filled, by
// CGSMSSendHandler.
//
// Message ids are fixed-width text the caller generates and increments
// (the dimension, world and server digits, then a zero-padded counter);
// the tables allocate nothing.
class SMSMessageRepository {
public:
    virtual ~SMSMessageRepository() {}

    // Open the relay connection. Called once, before any statement.
    virtual void open(const std::string& host, const std::string& db, const std::string& user,
                      const std::string& password, uint port) = 0;

    // Drop the open connection and open another to the same server. The
    // reconnect names no port, so a relay listening anywhere but the
    // client library's default is reachable only until the first
    // failure.
    virtual void reopen(const std::string& host, const std::string& db, const std::string& user,
                        const std::string& password) = 0;

    // MAX(mid) over this server's ids: the rows whose id starts with the
    // three digits and is keySize characters long. False when the read
    // answered no row; on true maxID carries the id as the driver
    // returned it — MAX() over no matching row is one NULL, which the
    // driver hands back as an empty string, so the caller accepts the
    // value only at exactly keySize characters.
    virtual bool loadMaxMessageID(int dimensionDigit, int worldDigit, int serverDigit, int keySize,
                                  std::string& maxID) = 0;

    // The message row, stamped now(). True when a row was inserted; the
    // caller queues the message and moves to the next id only then.
    // target, toName and callback are interpolated raw; body arrives
    // already escaped by the caller.
    virtual bool insertMessage(const std::string& mid, const std::string& target, const std::string& toName,
                               const std::string& callback, const std::string& body) = 0;

    // The send-queue row naming a message that is already inserted.
    virtual void enqueue(const std::string& mid) = 0;

    // A statement against the relay so its server does not time the idle
    // connection out.
    virtual void keepAlive() = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSMSMessageRepository.cpp.
SMSMessageRepository& defaultSMSMessageRepository();

#endif
