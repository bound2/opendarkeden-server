//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectGrayDarkness.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectGrayDarkness.h"

#include "Creature.h"
#include "Tile.h"
#include "Zone.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

EffectGrayDarkness::EffectGrayDarkness(Zone* pZone, ZoneCoord_t ZoneX, ZoneCoord_t ZoneY)

{
    __BEGIN_TRY

    m_pZone = pZone;
    m_X = ZoneX;
    m_Y = ZoneY;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// EffectGrayDarkness::affectCreature()
// bAffectByMove가 false면 제자리에서 당하는 것이므로..
// 외부의 SkillOK에서 modify info를 보낸다. 따라서 GCModifyInformation을
// 보낼 필요가 없다.
// pTarget의 시야가 변한  경우 true를 return
//////////////////////////////////////////////////////////////////////////////
/*bool EffectGrayDarkness::affectObject(Object* pTarget, bool bAffectByMove)

{
    __BEGIN_TRY

    bool bAffected = false;
    if (pTarget->getObjectClass() == Object::OBJECT_CLASS_CREATURE)
    {
        Creature* pTargetCreature = dynamic_cast<Creature*>(pTarget);
        // 이미 걸려있는 경우에는 다시 걸지 않는다.
        if (pTargetCreature->isFlag(Effect::EFFECT_CLASS_GRAY_DARKNESS))
        {
            return false;
        }
        // 슬레이어일 경우에만 걸린다.
        if (pTargetCreature->isSlayer()||pTargetCreature->isOusters())
        {
            bAffected = true;
            pTargetCreature->setFlag(Effect::EFFECT_CLASS_GRAY_DARKNESS);
        }
    }

    return bAffected;

    __END_CATCH
}

void EffectGrayDarkness::unaffectObject(Object* pTarget, bool bUnaffectByMove)

{
    __BEGIN_TRY

    Assert(pTarget != NULL);

    if (pTarget->getObjectClass() == Object::OBJECT_CLASS_CREATURE)
    {
        Creature* pTargetCreature = dynamic_cast<Creature*>(pTarget);

        if (pTargetCreature->isSlayer()||pTargetCreature->isOusters())
        {
            pTargetCreature->removeFlag(Effect::EFFECT_CLASS_GRAY_DARKNESS);
        }
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// EffectGrayDarkness::affect()
//////////////////////////////////////////////////////////////////////////////
void EffectGrayDarkness::affect(Creature* pTargetCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// EffectGrayDarkness::affect()
//////////////////////////////////////////////////////////////////////////////
void EffectGrayDarkness::affect(Zone* pZone , ZoneCoord_t x , ZoneCoord_t y , Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}
*/
//////////////////////////////////////////////////////////////////////////////
// unaffect()
//////////////////////////////////////////////////////////////////////////////
void EffectGrayDarkness::unaffect()

{
    __BEGIN_TRY

    // cout << "EffectGrayDarkness " << "unaffect BEGIN" << endl;

    Tile& tile = m_pZone->getTile(m_X, m_Y);

    // unaffect creatures on tile
    //	const forward_list<Object*>& oList = tile.getObjectList();
    //	for (forward_list<Object*>::const_iterator itr = oList.begin(); itr != oList.end(); itr++)
    //	{
    //		if (*itr != this)
    //		{
    //			EffectGrayDarkness::unaffectObject((Object*)(*itr), false);
    //		}
    //	}

    tile.deleteEffect(m_ObjectID);

    // cout << "EffectGrayDarkness " << "unaffect END" << endl;

    __END_CATCH
}

/*void EffectGrayDarkness::unaffect(Zone* pZone , ZoneCoord_t x , ZoneCoord_t y , Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}*/

string EffectGrayDarkness::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectGrayDarkness(" << "DayTime:" << m_Deadline.tv_sec << ")";
    return msg.toString();

    __END_CATCH
}
