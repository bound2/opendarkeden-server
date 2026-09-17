//////////////////////////////////////////////////////////////////////////////
// Filename    : GCTeachSkillInfo.cpp
// Description :
// The first packet sent to the player when an NPC is about to teach a skill.
// It describes the range of skills the NPC can teach and is
// the packet used for that.
//////////////////////////////////////////////////////////////////////////////

#include "GCTeachSkillInfo.h"

//////////////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////////////
void GCTeachSkillInfo::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_DomainType);
    iStream.read(m_TargetLevel);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////////////
void GCTeachSkillInfo::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_DomainType);
    oStream.write(m_TargetLevel);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCTeachSkillInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    string domain;

    switch (m_DomainType) {
    case SKILL_DOMAIN_BLADE:
        domain = "BLADE";
        break;
    case SKILL_DOMAIN_SWORD:
        domain = "SWORD";
        break;
    case SKILL_DOMAIN_GUN:
        domain = "GUN";
        break;
    // case SKILL_DOMAIN_RIFLE:   domain = "RIFLE";   break;
    case SKILL_DOMAIN_ENCHANT:
        domain = "ENCHANT";
        break;
    case SKILL_DOMAIN_HEAL:
        domain = "HEAL";
        break;
    case SKILL_DOMAIN_ETC:
        domain = "ETC";
        break;
    case SKILL_DOMAIN_VAMPIRE:
        domain = "VAMPIRE";
        break;
    default:
        domain = "UNKNOWN";
        break;
    }

    msg << "GCTeachSkillInfo(" << "DomainType:" << domain << "," << "TargetLevel:" << (int)m_TargetLevel << ")";

    return msg.toString();

    __END_CATCH
}
