//////////////////////////////////////////////////////////////////////
//
// Filename    : VampireSkillInfo.h
// Written By  : elca@ewestsoft.com
// Description :  Skill information
//
//////////////////////////////////////////////////////////////////////

#ifndef __VAMPIRE_SKILL_INFO_H__
#define __VAMPIRE_SKILL_INFO_H__

// include files
#include "Exception.h"
#include "PCSkillInfo.h"
#include "Packet.h"
#include "SubVampireSkillInfo.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class VampireSkillInfo;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class VampireSkillInfo : public PCSkillInfo {
public:
    // constructor
    VampireSkillInfo();

    // destructor
    ~VampireSkillInfo() noexcept;

public:
    // Read data from the input stream (buffer) and initialise the
    // packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getSize();

    // The skill count travels in a BYTE and the max budgets this many.
    static constexpr size_t kMaxSkills = 120;

    static constexpr uint getMaxSize() {
        return szBYTE + szBYTE + (SubVampireSkillInfo::getMaxSize() * kMaxSkills);
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
        return (BYTE)m_SubVampireSkillInfoList.size();
    }

    // add / delete / clear S List
    void addListElement(SubVampireSkillInfo* pSubVampireSkillInfo) {
        if (m_SubVampireSkillInfoList.size() >= kMaxSkills)
            throw InvalidProtocolException("too many vampire skills");
        m_SubVampireSkillInfoList.push_back(pSubVampireSkillInfo);
    }

    // ClearList
    void clearList();

    // pop front Element in Status List
    SubVampireSkillInfo* popFrontListElement() {
        SubVampireSkillInfo* TempSubVampireSkillInfo = m_SubVampireSkillInfoList.front();
        m_SubVampireSkillInfoList.pop_front();
        return TempSubVampireSkillInfo;
    }

private:
    // Whether a new skill can be learned
    bool m_bLearnNewSkill;

    // SubVampireSkillInfo List
    list<SubVampireSkillInfo*> m_SubVampireSkillInfoList;
};

#endif
