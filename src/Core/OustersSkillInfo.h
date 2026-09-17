//////////////////////////////////////////////////////////////////////
//
// Filename    : OustersSkillInfo.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __OUSTERS_SKILL_INFO_H__
#define __OUSTERS_SKILL_INFO_H__

// include files
#include "Exception.h"
#include "PCSkillInfo.h"
#include "Packet.h"
#include "SubOustersSkillInfo.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class OustersSkillInfo;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class OustersSkillInfo : public PCSkillInfo {
public:
    // constructor
    OustersSkillInfo();

    // destructor
    ~OustersSkillInfo();

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getSize();

    // The skill count travels in a BYTE and the max budgets this many.
    static constexpr size_t kMaxSkills = 120;

    static constexpr uint getMaxSize() {
        return szBYTE + szBYTE + (SubOustersSkillInfo::getMaxSize() * kMaxSkills);
    }

    // get packet's debug string
    string toString() const;

    // get / set New Skill
    bool isLearnNewSkill() const {
        return m_bLearnNewSkill;
    }
    void setLearnNewSkill(bool NewSkill) {
        m_bLearnNewSkill = NewSkill;
    }

    // get ListNumber: the count write() emits is the list itself.
    BYTE getListNum() const {
        return (BYTE)m_SubOustersSkillInfoList.size();
    }

    // add / delete / clear S List
    void addListElement(SubOustersSkillInfo* pSubOustersSkillInfo) {
        if (m_SubOustersSkillInfoList.size() >= kMaxSkills)
            throw InvalidProtocolException("too many ousters skills");
        m_SubOustersSkillInfoList.push_back(pSubOustersSkillInfo);
    }

    // ClearList
    void clearList();

    // pop front Element in Status List
    SubOustersSkillInfo* popFrontListElement() {
        SubOustersSkillInfo* TempSubOustersSkillInfo = m_SubOustersSkillInfoList.front();
        m_SubOustersSkillInfoList.pop_front();
        return TempSubOustersSkillInfo;
    }

private:
    // Whether a new skill can be learned
    bool m_bLearnNewSkill;

    // SubOustersSkillInfo List
    list<SubOustersSkillInfo*> m_SubOustersSkillInfoList;
};

#endif
