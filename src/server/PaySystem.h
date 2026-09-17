//////////////////////////////////////////////////////////////////////////////
// Filename   : PaySystem.h
// Written by : sigi
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __PAY_SYSTEM_H__
#define __PAY_SYSTEM_H__

#include "Exception.h"
#include "Timeval.h"
#include "Types.h"
#include "VSDateTime.h"

class Statement;


//////////////////////////////////////////////////////////////////////////////
// Rate plan
//////////////////////////////////////////////////////////////////////////////
enum PayPlayType {
    PAY_PLAY_TYPE_PERSON, // personal rate
    PAY_PLAY_TYPE_PCROOM, // PC room rate
    PAY_PLAY_TYPE_ETC,    // other (just put in)
    PAY_PLAY_TYPE_MAX
};

enum PayType {
    PAY_TYPE_FREE,   // free user
    PAY_TYPE_PERIOD, // monthly user
    PAY_TYPE_TIME,   // time-based user
    PAY_TYPE_POST,   // post-paid time-based user
    PAY_TYPE_MAX
};

enum PayIPType {
    PAY_IP_TYPE_ALL,    // every IP
    PAY_IP_TYPE_LIMIT,  // a limit on the number of concurrent connections
    PAY_IP_TYPE_ASSIGN, // a specific IP
    PAY_IP_TYPE_MAX
};

enum FamilyPayPartyType {
    FAMILY_PAY_PARTY_TYPE_NONE,
    FAMILY_PAY_PARTY_TYPE_FREE_PASS,     // a family-plan party may enter a pay zone
    FAMILY_PAY_PARTY_TYPE_FREE_PASS_END, // end of the family-plan party pay-zone entry
};

#define PAY_PLAY_FLAG_ALL 0xFFFF
#define PAY_PLAY_FLAG_ZONE 0x0001


// GamePlayer::isPayPlaying() answers true for every player, so the
// pay/premium gates - zone entry, premium-zone item rules such as
// Slayer::isRealWearing() refusing a multi-option weapon, portal and
// potion use - all pass. This server has no billing backend.


//////////////////////////////////////////////////////////////////////////////
// class PaySystem
//////////////////////////////////////////////////////////////////////////////

class PaySystem {
public:
    PaySystem();
    ~PaySystem();

    PayPlayType getPayPlayType() const {
        return m_PayPlayType;
    }
    void setPayPlayType(PayPlayType ppt) {
        m_PayPlayType = ppt;
    }

    uint getPCRoomID() const {
        return m_PCRoomID;
    }
    void setPCRoomID(uint id) {
        m_PCRoomID = id;
    }

    PayType getPayType() const {
        return m_PayType;
    }
    void setPayType(PayType pt) {
        m_PayType = pt;
    }

    PayIPType getPayIPType() const {
        return m_PayIPType;
    }
    void setPayIPType(PayIPType pipt) {
        m_PayIPType = pipt;
    }

    uint getPayPlayFlag() const {
        return m_PayPlayFlag;
    }
    void setPayPlayFlag(uint ppf) {
        m_PayPlayFlag = ppf;
    }
    bool hasPayPlayFlag(uint flag) const {
        return m_PayPlayFlag & flag;
    }

    void setPayPlayAvailableDateTime(const string& pat);
    const VSDateTime& getPayPlayAvailableDateTime() const {
        return m_PayPlayAvailableDateTime;
    } // until when can it be played?

    void setFamilyPayPlayAvailableDateTime(const string& pat);
    const VSDateTime& getFamilyPayPlayAvailableDateTime() const {
        return m_FamilyPayPlayAvailableDateTime;
    } // until when can it be played?

    void setPayStartAvailableDateTime(const string& pat);
    const VSDateTime& getPayStartAvailableDateTime() const {
        return m_PayStartAvailableDateTime;
    } // from when can it be played?

    Timeval getPayPlayTime(const Timeval& currentTime) const {
        return timediff(m_PayPlayStartTime, currentTime);
    }
    void setPayPlayStartTime(const Timeval& tv) {
        m_PayPlayStartTime = tv;
    }

    void setPayPlayAvailableHours(int h) {
        m_PayPlayAvailableHours = h;
    }
    int getPayPlayAvailableHours() const {
        return m_PayPlayAvailableHours;
    }


public:
    // Check whether it can be used
    bool checkPayPlayAvailable();

    // Just set the value for now
    void setPayPlayValue(PayType payType, const string& payPlayDate, int payPlayHours, uint payPlayFlag,
                         const string& familyPayPlayDate = "");

    // pay starts
    bool loginPayPlay(PayType payType, const string& payPlayDate, int payPlayHours, uint payPlayFlag, const string& ip,
                      const string& playerID);

    // pay starts
    bool loginPayPlay(const string& ip, const string& playerID);

    // pay time update and so on..
    bool updatePayPlayTime(const string& playerID, const VSDateTime& currentDateTime, const Timeval& currentTime);

    // pay ends
    void logoutPayPlay(const string& playerID, bool bClear = false, bool bDecreaseTime = true);

    //
    bool isPayPlaying() const {
        return m_PayPlayStartTime.tv_sec != 0;
    }

    void setPremiumPlay(bool bPremium = true) {
        m_bPremiumPlay = bPremium;
    }
    bool isPremiumPlay() const {
        return m_bPremiumPlay;
    }

    void setFamilyPayAvailable(bool bFamilyPayAvailable = true) {
        m_bFamilyPayAvailable = bFamilyPayAvailable;
    }
    bool isFamilyPayAvailable() const {
        return m_bFamilyPayAvailable;
    }

    void setFamilyPayPartyType(FamilyPayPartyType familyPayPartyType) {
        m_FamilyPayPartyType = familyPayPartyType;
    }
    bool isFamilyFreePass() const {
        return m_FamilyPayPartyType == FAMILY_PAY_PARTY_TYPE_FREE_PASS;
    }
    bool isFamilyFreePassEnd() const {
        return m_FamilyPayPartyType == FAMILY_PAY_PARTY_TYPE_FREE_PASS_END;
    }

    void setPCRoomPlay(bool bPCRoom = true) {
        m_bPCRoomPlay = bPCRoom;
    }
    bool isPCRoomPlay() const {
        return m_bPCRoomPlay;
    }

    // Is this a personal flat-rate paying user?
    static bool isPayPlayingPeriodPersonal(const string& PlayerID);

    bool isPlayInPayPCRoom(const string& ip, const string& playerID);

protected:
    // Handling for a PC room
    bool loginPayPlayPCRoom(const string& ip, const string& playerID);
    void logoutPayPlayPCRoom(const string& playerID);

    // When the time is reduced under a metered plan..
    void decreasePayPlayTime(const string& playerID, uint mm);
    void decreasePayPlayTimePCRoom(uint mm);

    // An odd flat-rate code
    void increasePayPlayTimePCRoom(uint mm);

    // Deletes every Pay entry and makes it a free user.
    void clearPayPlayDateTime(const string& playerID);

protected:
    bool m_bSetPersonValue; // was the value set.

    PayPlayType m_PayPlayType; // personal/PC room..
    ObjectID_t m_PCRoomID;     // PC room ID
    uint m_UserLimit;          // user count limit (metered)
    uint m_UserMax;            // maximum user count (flat rate)

    PayType m_PayType;     // free/time/period/other..
    PayIPType m_PayIPType; // the IP type for a PC room

    VSDateTime m_PayStartAvailableDateTime;      // from when can it be played?
    VSDateTime m_PayPlayAvailableDateTime;       // until when can it be played?
    int m_PayPlayAvailableHours;                 // the hours available (for a time-based plan)
    Timeval m_PayPlayStartTime;                  // how long it has been played since connecting
    VSDateTime m_FamilyPayPlayAvailableDateTime; // until when can Family Pay play be used?

    uint m_PayPlayFlag; // the services that may be used

    bool m_bPremiumPlay;
    bool m_bPCRoomPlay; // is this a PC room user?
        // is it played in a PC room under a paying plan, whatever the PC room user limit?

    bool m_bFamilyPayAvailable;              // is this a family-plan user?
    FamilyPayPartyType m_FamilyPayPartyType; // the family-plan party type applied
};

#endif
