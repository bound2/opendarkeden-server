#ifndef __SIEGE_MANAGER_H__
#define __SIEGE_MANAGER_H__

#include "Exception.h"
#include "Types.h"

class PlayerCreature;
class MonsterCorpse;
class Item;

class SiegeManager {
public:
    void init();
    void init(ZoneID_t zoneID);
    // Set up and empty the siege zone zoneID. Both change the zone's
    // creatures, so they run on its group's thread: a siege war posts them
    // there (de::war::postToZone), and a GM's command runs in the zone itself.
    void start(ZoneID_t zoneID);
    void reset(ZoneID_t zoneID);

    ZoneID_t getSiegeZoneID(ZoneID_t castleZoneID);

    static SiegeManager& Instance() {
        static SiegeManager theInstance;
        return theInstance;
    }

    void putItem(PlayerCreature* pPC, MonsterCorpse* pCorpse, Item* pItem);

    bool isSiegeZone(ZoneID_t zID);

private:
};

#endif
