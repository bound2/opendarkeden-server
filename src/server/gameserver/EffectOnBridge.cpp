//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectOnBridge.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectOnBridge.h"

#include "Creature.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "GCStatusCurrentHP.h"
#include "Monster.h"
#include "Ousters.h"
#include "Player.h"
#include "Slayer.h"
#include "StringStream.h"
#include "Vampire.h"
#include "Zone.h"
#include "ZoneUtil.h"
#include "repository/ZoneInfoRepository.h"
#include "skill/EffectBloodDrain.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectOnBridge::EffectOnBridge(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y) {
    __BEGIN_TRY

    m_pZone = pZone;
    setXY(x, y);
    setBroadcastingEffect(false);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectOnBridge::affect()

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectOnBridge::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectOnBridge::unaffect()

{
    __BEGIN_TRY

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectOnBridge::unaffect(Creature* pCreature)

    {__BEGIN_TRY __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectOnBridge::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectOnBridge(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
void EffectOnBridgeLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    vector<ZoneEffectRow> rows =
        defaultZoneInfoRepository().loadZoneEffectRects(pZone->getZoneID(), (int)Effect::EFFECT_CLASS_ON_BRIDGE);

    for (size_t r = 0; r < rows.size(); r++) {
        ZoneCoord_t left = rows[r].left;
        ZoneCoord_t top = rows[r].top;
        ZoneCoord_t right = rows[r].right;
        ZoneCoord_t bottom = rows[r].bottom;
        // rows[r].value1 / value2 / value3 are selected and ignored, as before

        VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

        for (int X = left; X <= right; X++)
            for (int Y = top; Y <= bottom; Y++) {
                if (rect.ptInRect(X, Y)) {
                    Tile& tile = pZone->getTile(X, Y);
                    if (tile.canAddEffect()) {
                        EffectOnBridge* pEffect = new EffectOnBridge(pZone, X, Y);

                        // Tile-level effects should NOT be added to Zone's EffectManager.
                        // They are permanent (deadline=99999999) and managed by Tile itself.
                        // Adding them to Zone's EffectManager causes severe CPU overhead
                        // because heartbeat() iterates through all effects every tick.
                        pZone->registerObject(pEffect);
                        // pZone->addEffect(pEffect);  // REMOVED: Don't add permanent tile effects to Zone
                        tile.addEffect(pEffect);
                    }
                }
            }
    }

    __END_CATCH
}

EffectOnBridgeLoader* g_pEffectOnBridgeLoader = NULL;
