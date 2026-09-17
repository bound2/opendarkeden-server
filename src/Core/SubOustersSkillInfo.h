//----------------------------------------------------------------------
//
// Filename    : SubOustersSkillInfo.h
// Written By  :
// Description :
//
//----------------------------------------------------------------------

#ifndef __SUB_OUSTERS_SKILL_INFO_H__
#define __SUB_OUSTERS_SKILL_INFO_H__

// include files
#include "Exception.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Types.h"

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

class SubOustersSkillInfo {
public:
    // read data from socket input stream
    void read(SocketInputStream& iStream);

    // write data to socket output stream
    void write(SocketOutputStream& oStream) const;

    // get size of object
    uint getSize() const {
        return szSkillType + szExpLevel + szTurn + szTurn;
    }
    // get max size of object
    static constexpr uint getMaxSize() {
        return szSkillType + szExpLevel + szTurn + szTurn;
    }

    // get debug string
    string toString() const;

public:
    // get / set SkillType
    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

    // get /set Skill ExpLevel
    ExpLevel_t getExpLevel() const {
        return m_ExpLevel;
    }
    void setExpLevel(ExpLevel_t ExpLevel) {
        m_ExpLevel = ExpLevel;
    }

    // get / set Turn
    Turn_t getSkillTurn() const {
        return m_Interval;
    }
    void setSkillTurn(Turn_t SkillTurn) {
        m_Interval = SkillTurn;
    }

    // get / set CastingTime
    Turn_t getCastingTime() const {
        return m_CastingTime;
    }
    void setCastingTime(Turn_t CastingTime) {
        m_CastingTime = CastingTime;
    }

private:
    // Skill type
    SkillType_t m_SkillType;

    // Skill level
    ExpLevel_t m_ExpLevel;

    // Delay before it can be used again
    Turn_t m_Interval;

    // Casting time
    Turn_t m_CastingTime;
};

#endif
