////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionTeachSkill.cpp
// Written By  :
// Description :
// Action used when an NPC teaches a skill to the player.
// In practice the NPC only sends the skills it can teach as a packet;
// the remaining work is handled while the packet travels back and
// forth.
////////////////////////////////////////////////////////////////////////////////

#include "ActionTeachSkill.h"

#include "Creature.h"
#include "GCNPCResponse.h"
#include "GCTeachSkillInfo.h"
#include "GamePlayer.h"
#include "NPC.h"
#include "SkillInfo.h"
#include "Slayer.h"
#include "Vampire.h"

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ActionTeachSkill::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY

    try {
        // Read the domain type.
        string domainType = propertyBuffer.getProperty("DomainType");

        if (domainType == "BLADE")
            m_DomainType = SKILL_DOMAIN_BLADE;
        else if (domainType == "SWORD")
            m_DomainType = SKILL_DOMAIN_SWORD;
        else if (domainType == "GUN")
            m_DomainType = SKILL_DOMAIN_GUN;
        else if (domainType == "ENCHANT")
            m_DomainType = SKILL_DOMAIN_ENCHANT;
        else if (domainType == "HEAL")
            m_DomainType = SKILL_DOMAIN_HEAL;
        else if (domainType == "ETC")
            m_DomainType = SKILL_DOMAIN_ETC;
        else if (domainType == "VAMPIRE")
            m_DomainType = SKILL_DOMAIN_VAMPIRE;
        else
            throw Error("TeachSkill::read() : invalid skill type.");
    } catch (NoSuchElementException& nsee) {
        throw Error(nsee.toString());
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionTeachSkill::execute(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Assert(pCreature1 != NULL);
    Assert(pCreature2 != NULL);
    Assert(pCreature1->isNPC());
    Assert(pCreature2->isPC());

    // Send an OK packet to the client first.
    GCNPCResponse okpkt;
    Player* pPlayer = pCreature2->getPlayer();
    Assert(pPlayer != NULL);
    pPlayer->sendPacket(&okpkt);

    if (pCreature2->isSlayer())
        executeSlayer(pCreature1, pCreature2);
    else if (pCreature2->isVampire())
        executeVampire(pCreature1, pCreature2);
    else
        throw Error("ActionTeachSkill::execute() : unknown player creature");

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionTeachSkill::executeSlayer(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature2);
    Player* pPlayer = pCreature2->getPlayer();
    GCTeachSkillInfo teachinfo;

    Assert(pPlayer != NULL);

    // First check whether the skill can be learned.
    if (pSlayer->getGoalExp(m_DomainType) != 0) {
        // Build the packet...
        teachinfo.setDomainType(m_DomainType);
        teachinfo.setTargetLevel(0);
        pPlayer->sendPacket(&teachinfo);
        return;
    }

    Level_t DomainLevel = pSlayer->getSkillDomainLevel(m_DomainType);
    SkillType_t SkillType = g_pSkillInfoManager->getSkillTypeByLevel(m_DomainType, DomainLevel);


    // Build the packet...
    teachinfo.setDomainType(m_DomainType);
    teachinfo.setTargetLevel(SkillType);

    pPlayer->sendPacket(&teachinfo);

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionTeachSkill::executeVampire(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Vampire* pVampire = dynamic_cast<Vampire*>(pCreature2);


    Level_t DomainLevel = pVampire->getLevel();

    SkillType_t SkillType = g_pSkillInfoManager->getSkillTypeByLevel(m_DomainType, DomainLevel);


    // Build the packet...
    GCTeachSkillInfo teachinfo;
    teachinfo.setDomainType(m_DomainType);
    teachinfo.setTargetLevel(SkillType);

    // Send it.
    Player* pPlayer = pCreature2->getPlayer();
    Assert(pPlayer != NULL);
    pPlayer->sendPacket(&teachinfo);

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionTeachSkill::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionTeachSkill(" << ")";
    return msg.toString();

    __END_CATCH
}
