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
// 게임서버에서 클라이언트로 자신의 기술이 성공을 알려주기 위한 클래스
//
//////////////////////////////////////////////////////////////////////

class OustersSkillInfo : public PCSkillInfo {
public:
    // constructor
    OustersSkillInfo();

    // destructor
    ~OustersSkillInfo();

public:
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // 최적화시, 미리 계산된 정수를 사용한다.
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
    // New 스킬을 배울 수 있느냐 없느냐 정보
    bool m_bLearnNewSkill;

    // SubOustersSkillInfo List
    list<SubOustersSkillInfo*> m_SubOustersSkillInfoList;
};

#endif
