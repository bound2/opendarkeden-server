//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectSanctuary.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectSanctuary.h"

#include "Creature.h"
#include "Tile.h"
#include "Zone.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

EffectSanctuary::EffectSanctuary(Zone* pZone, ZoneCoord_t ZoneX, ZoneCoord_t ZoneY, ZoneCoord_t CenterX,
                                 ZoneCoord_t CenterY)

{
    __BEGIN_TRY

    m_pZone = pZone;
    m_X = ZoneX;
    m_Y = ZoneY;

    m_CenterX = CenterX;
    m_CenterY = CenterY;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// EffectSanctuary::affectCreature()
// bAffectByMove가 false면 제자리에서 당하는 것이므로..
// 외부의 SkillOK에서 modify info를 보낸다. 따라서 GCModifyInformation을
// 보낼 필요가 없다.
// pTarget의 시야가 변한  경우 true를 return
//////////////////////////////////////////////////////////////////////////////
bool EffectSanctuary::affectObject(Object* pTarget, bool bAffectByMove)

{
    __BEGIN_TRY

    return false;

    __END_CATCH
}

void EffectSanctuary::unaffectObject(Object* pTarget, bool bUnaffectByMove)

{
    __BEGIN_TRY


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// EffectSanctuary::affect()
//////////////////////////////////////////////////////////////////////////////
void EffectSanctuary::affect(Creature* pTargetCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// EffectSanctuary::affect()
//////////////////////////////////////////////////////////////////////////////
void EffectSanctuary::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// unaffect()
//////////////////////////////////////////////////////////////////////////////
void EffectSanctuary::unaffect()

{
    __BEGIN_TRY


    Tile& tile = m_pZone->getTile(m_X, m_Y);

    tile.deleteEffect(m_ObjectID);


    __END_CATCH
}

void EffectSanctuary::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

    {__BEGIN_TRY __END_CATCH}

string EffectSanctuary::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectSanctuary(" << "DayTime:" << m_Deadline.tv_sec << ")";
    return msg.toString();

    __END_CATCH
}
