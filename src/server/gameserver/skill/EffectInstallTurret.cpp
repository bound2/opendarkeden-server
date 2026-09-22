//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectInstallTurret.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectInstallTurret.h"

#include "GCModifyInformation.h"
#include "GCOtherModifyInfo.h"
#include "GCRemoveEffect.h"
#include "GCStatusCurrentHP.h"
#include "Monster.h"
#include "PacketUtil.h"
#include "Player.h"
#include "Slayer.h"
#include "Vampire.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectInstallTurret::EffectInstallTurret(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    Assert(pCreature->isSlayer());

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectInstallTurret::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectInstallTurret::unaffect()

{
    __BEGIN_TRY


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectInstallTurret::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);
    Assert(pCreature->isSlayer()); // Only a Slayer can be affected.

    pCreature->removeFlag(Effect::EFFECT_CLASS_INSTALL_TURRET);

    Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

    SLAYER_RECORD prev;
    pSlayer->getSlayerRecord(prev);
    pSlayer->initAllStat();
    pSlayer->sendRealWearingInfo();
    pSlayer->sendModifyInfo(prev);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_INSTALL_TURRET);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    GCOtherModifyInfo gcOtherModifyInfo;
    makeGCOtherModifyInfo(&gcOtherModifyInfo, pSlayer, &prev);
    pZone->broadcastPacket(pSlayer->getX(), pSlayer->getY(), &gcOtherModifyInfo, pSlayer);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectInstallTurret::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectInstallTurret(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
