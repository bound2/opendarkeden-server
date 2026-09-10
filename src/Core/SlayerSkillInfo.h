//////////////////////////////////////////////////////////////////////
//
// Filename    : SlayerSkillInfo.h
// Written By  : elca@ewestsoft.com
// Description :  Slayer skill information
//
//////////////////////////////////////////////////////////////////////

#ifndef __SLAYER_SKILL_INFO_H__
#define __SLAYER_SKILL_INFO_H__

// include files
#include "Exception.h"
#include "PCSkillInfo.h"
#include "Packet.h"
#include "SubSlayerSkillInfo.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class SlayerSkillInfo;
//
// Holds a Slayer's skill information to be delivered to the client.
//
//////////////////////////////////////////////////////////////////////

class SlayerSkillInfo : public PCSkillInfo {
public:
    // constructor
    SlayerSkillInfo();

    // destructor
    ~SlayerSkillInfo() noexcept;

public:
    // Initialize the packet by reading data from the input stream.
    virtual void read(SocketInputStream& iStream);

    // Serialize the packet into the output stream.
    virtual void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // Serialized size varies with the contained skill list.
    PacketSize_t getSize();

    // The skill count travels in a BYTE and the max budgets this many.
    static constexpr size_t kMaxSkills = 255;

    static constexpr uint getMaxSize() {
        return szBYTE + szSkillDomainType + szBYTE + (SubSlayerSkillInfo::getMaxSize() * kMaxSkills);
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

    // get /set DomainType
    SkillDomainType_t getDomainiType() const {
        return m_DomainType;
    }
    void setDomainType(SkillDomainType_t DomainType) {
        m_DomainType = DomainType;
    }

    // get ListNumber: the count write() emits is the list itself.
    BYTE getListNum() const {
        return (BYTE)m_SubSlayerSkillInfoList.size();
    }

    // add / delete / clear S List
    void addListElement(SubSlayerSkillInfo* pSubSlayerSkillInfo) {
        if (m_SubSlayerSkillInfoList.size() >= kMaxSkills)
            throw InvalidProtocolException("too many slayer skills");
        m_SubSlayerSkillInfoList.push_back(pSubSlayerSkillInfo);
    }

    // ClearList
    void clearList();

    // pop front Element in Status List
    SubSlayerSkillInfo* popFrontListElement() {
        SubSlayerSkillInfo* TempSubSlayerSkillInfo = m_SubSlayerSkillInfoList.front();
        m_SubSlayerSkillInfoList.pop_front();
        return TempSubSlayerSkillInfo;
    }

private:
    // Tracks whether there is a newly learnable skill
    bool m_bLearnNewSkill;

    SkillDomainType_t m_DomainType;

    // SubSlayerSkillInfo List
    list<SubSlayerSkillInfo*> m_SubSlayerSkillInfoList;
};

#endif
