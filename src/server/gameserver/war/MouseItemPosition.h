#ifndef __MOUSE_ITEM_POSITION_H__
#define __MOUSE_ITEM_POSITION_H__

#include "GlobalItemPosition.h"
#include "Types.h"

class Creature;
class PlayerCreature;
class Zone;

class MouseItemPosition : public GlobalItemPosition {
public:
    MouseItemPosition() : GlobalItemPosition(POS_TYPE_MOUSE) {
        m_bSetZone = false;
    }
    ~MouseItemPosition() {}

public:
    Item* popItem(bool bLock = true) override;
    Item* popItemFrom(PlayerCreature& pc) override;
    Zone* getZone() override;

    string getOwnerName() const {
        return m_OwnerName;
    }
    void setOwnerName(const string& ownerName) {
        m_bSetZone = false;
        m_OwnerName = ownerName;
    }

public:
    string toString() const override;

protected:
    Item* popItem_LOCKED();
    Item* popItem_UNLOCKED();
    Creature* findCreature();
    Zone* getZoneByCreature(Creature* pCreature);
    Item* popItem_CORE(PlayerCreature* pPC);

private:
    string m_OwnerName;

    // For getZone......
    Zone* m_pZone;
    bool m_bSetZone;
};

#endif // __MOUSE_ITEM_POSITION_H__
