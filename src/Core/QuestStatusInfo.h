#ifndef __QUEST_STATUS_INFO_H__
#define __QUEST_STATUS_INFO_H__

#include <list>

#include "Packet.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Types.h"
#include "WireString.h"

#define MAX_MISSION_NUM 100

struct MissionInfo {
    enum Status {
        HIDE = 0, // Not shown
        CURRENT,  // The mission to do now (in progress)
        SUCCESS,  // A mission that has already succeeded
        FAIL,     // Failed
    };

    BYTE m_Condition = 0; // Which condition it is in 0 : Happen, 1 : Complete, 2 : Fail, 3 : Reward
    WORD m_Index = 0;     // Which element of that condition it is
    BYTE m_Status = 0;    // Current state

    string m_StrArg;    // String to print
    DWORD m_NumArg = 0; // Number to print

    MissionInfo() {}
    virtual ~MissionInfo() {}

    void read(SocketInputStream& iStream) {
        iStream.read(m_Condition);
        iStream.read(m_Index);
        iStream.read(m_Status);

        de::wire::readString(iStream, m_StrArg, {0, de::wire::kMaxByteStringLength}, "StrArg");

        iStream.read(m_NumArg);
    }

    void write(SocketOutputStream& oStream) const {
        oStream.write(m_Condition);
        oStream.write(m_Index);
        oStream.write(m_Status);

        de::wire::writeString(oStream, m_StrArg, {0, de::wire::kMaxByteStringLength}, "StrArg");

        oStream.write(m_NumArg);
    }

    PacketSize_t getSize() const {
        return szBYTE + szWORD + szBYTE + de::wire::stringWireSize(m_StrArg) + szDWORD;
    }
    static constexpr PacketSize_t getMaxSize() {
        return szBYTE + szWORD + szBYTE + szBYTE + 255 + szDWORD;
    }
};

class QuestStatusInfo {
public:
    enum Status {
        CANNOT = 0, // Not possible yet
        CAN_ACCEPT, // Can be started
        DOING,      // In progress
        SUCCESS,    // Succeeded (not rewarded yet)
        COMPLETE,   // Complete (rewarded, cannot be done again)
        FAIL,       // Failed (cannot be done again)
        CAN_REPLAY, // Can be done again
    };

    QuestStatusInfo(DWORD qID) : m_QuestID(qID), m_Status(CANNOT) {}
    ~QuestStatusInfo();

    // The record owns the missions it holds.
    void clearMissions();

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketSize_t getSize() const;
    static constexpr PacketSize_t getMaxSize() {
        return szDWORD + szBYTE + szBYTE + MissionInfo::getMaxSize() * MAX_MISSION_NUM;
    }

protected:
    DWORD m_QuestID;
    BYTE m_Status; // See the enum above
    list<MissionInfo*> m_Missions;
};

#endif
