#ifndef __SMS_ADDRESS_REPOSITORY_H__
#define __SMS_ADDRESS_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the SMSAddressBook table (task 3.2): a
// character's phone-number book, one row per entry keyed by
// (eID, OwnerID). The SMSAddressBook class owns the in-memory map and
// the eID allocation; this seam moves rows.
//
// What load() returns — each field typed to the driver getter the
// inline code called: eID through getInt (the column is int unsigned;
// the class stores it as DWORD after the int has been read), the three
// texts through getString.
struct SMSAddressRow {
    int eID;
    std::string characterName;
    std::string customName;
    std::string number;
};

class SMSAddressRepository {
public:
    virtual ~SMSAddressRepository() {}

    // Every entry the owner has.
    virtual std::vector<SMSAddressRow> load(const std::string& ownerName) = 0;

    // SMSAddressBook::addAddressElement — a new entry. The eID is the
    // element's DWORD id; the (eID, OwnerID) primary key refuses a
    // repeat.
    virtual void insert(const std::string& ownerName, DWORD eID, const std::string& characterName,
                        const std::string& customName, const std::string& number) = 0;

    // SMSAddressBook::removeAddressElement.
    virtual void remove(const std::string& ownerName, DWORD eID) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSMSAddressRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
SMSAddressRepository& defaultSMSAddressRepository();

#endif
