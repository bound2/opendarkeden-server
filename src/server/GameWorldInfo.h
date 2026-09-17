//----------------------------------------------------------------------
//
// Filename    : GameWorldInfo.h
// Written By  : Reiot
// Description : the information the login server holds about each game server
//
//----------------------------------------------------------------------

#ifndef __GAME_WORLD_INFO_H__
#define __GAME_WORLD_INFO_H__

// include files
#include "Exception.h"
#include "StringStream.h"
#include "Types.h"


//----------------------------------------------------------------------
//
// class GameWorldInfo;
//
// A class holding each game server's information read from the GameWorldInfo
// table in the GAME DB.
//
//----------------------------------------------------------------------

class GameWorldInfo {
public:
    // get/set GameWorldID
    WorldID_t getID() const {
        return m_ID;
    }
    void setID(WorldID_t ID) {
        m_ID = ID;
    }

    // get/set host name
    string getName() const {
        return m_Name;
    }
    void setName(string Name) {
        m_Name = Name;
    }

    // get/set World Status
    WorldStatus getStatus() const {
        return m_Status;
    }
    void setStatus(WorldStatus status) {
        m_Status = status;
    }

    // get debug string
    string toString() const {
        StringStream msg;
        msg << "GameWorldInfo(" << "WorldID: " << (int)m_ID << ",Name:" << m_Name << ")";
        return msg.toString();
    }

private:
    // GameWorld ID
    WorldID_t m_ID;

    // GameWorld Process's nick name
    string m_Name;

    // GameWorld status
    WorldStatus m_Status;
};

#endif
