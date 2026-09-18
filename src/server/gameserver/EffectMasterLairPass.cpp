//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectMasterLairPass.cpp
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectMasterLairPass.h"

#include "Creature.h"
#include "GCRemoveEffect.h"
#include "GCSystemMessage.h"
#include "Player.h"
#include "ZoneInfoManager.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectMasterLairPass::EffectMasterLairPass(Creature* pCreature, ZoneID_t zoneID)

{
    __BEGIN_TRY

    m_ZoneID = zoneID;

    setTarget(pCreature);

    // Server-only effect.
    m_bBroadcastingEffect = false;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMasterLairPass::affect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    affect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMasterLairPass::affect(Creature* pCreature)

{
    __BEGIN_TRY


    __END_CATCH
}

void EffectMasterLairPass::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMasterLairPass::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // cout << "EffectMasterLairPass" << "unaffect BEGIN" << endl;

    Assert(pCreature != NULL);

    // To restore the attributes properly, clear the flag and
    // call initAllStat.
    pCreature->removeFlag(Effect::EFFECT_CLASS_MASTER_LAIR_PASS);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    /*
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_MASTER_LAIR_PASS);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    */

    // cout << "EffectMasterLairPass" << "unaffect END" << endl;

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectMasterLairPass::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectMasterLairPass(" << "ObjectID:" << getObjectID() << ",ZoneID:" << m_ZoneID << ")";
    return msg.toString();

    __END_CATCH
}
