//--------------------------------------------------------------------------------
//
// Filename   : TelephoneCenter.h
// Written By : elca
//
//--------------------------------------------------------------------------------

#ifndef __TELEPHONE_CENTER_H__
#define __TELEPHONE_CENTER_H__

// include files
#include <unordered_map>

#include "Exception.h"
#include "Mutex.h"
#include "Slayer.h"
#include "Types.h"

//--------------------------------------------------------------------------------
// class TelephoneCenter;
//
// Global manager object of the game server that gives access to a PC object by
// the PC's phone number. Internally uses an unordered_map to speed up the lookup.
//
//--------------------------------------------------------------------------------
class TelephoneCenter {
public:
    // add creature to unordered_map
    // execute just once at PC's login
    void addSlayer(Slayer* pSlayer);

    // delete creature from unordered_map
    // execute just once at PC's logout
    void deleteSlayer(PhoneNumber_t PhoneNumber);

    // get creature with PC-name
    Slayer* getSlayer(PhoneNumber_t PhoneNumber) const;

private:
    unordered_map<PhoneNumber_t, const Slayer*> m_PCs;

    mutable Mutex m_Mutex;
};

#endif
