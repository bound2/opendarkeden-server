//////////////////////////////////////////////////////////////////////////////
// Filename     : CombatInfoManager.h
// Written by   : bezz
// Description  : Sets the values tied to the outcome of a war.
//////////////////////////////////////////////////////////////////////////////

#ifndef __COMBAT_INFO_MANAGER_H__
#define __COMBAT_INFO_MANAGER_H__

#include "Assert.h"
#include "Exception.h"
#include "Mutex.h"
#include "Relic.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class CombatInfoManager
//
// Holds the current owner of each Relic and computes the
// resulting bonus and penalty values.
//////////////////////////////////////////////////////////////////////////////

class CombatInfoManager {
public:
    ///////////////////////////////////////////////////////////////////
    // Relic Owner
    // The side that owns the Relic and so receives the bonus or penalty
    ///////////////////////////////////////////////////////////////////
    enum RelicOwner { RELIC_OWNER_NULL, RELIC_OWNER_SLAYER, RELIC_OWNER_VAMPIRE };

public:
    // Constructor
    CombatInfoManager();

    // Initialize the bonus and penalty values.
    void initModify();

    // Compute the bonus and penalty.
    void computeModify();

    // Set the Relic owner.
    void setRelicOwner(int index, RelicOwner relicOwner);


    // Return the bonus and penalty values
    int getSlayerHPModify() const {
        return m_SlayerHPModify;
    }
    int getSlayerToHitModify() const {
        return m_SlayerToHitModify;
    }
    int getVampireHPModify() const {
        return m_VampireHPModify;
    }
    int getVampireToHitModify() const {
        return m_VampireToHitModify;
    }

    bool isSlayerBonus() const {
        return m_bSlayerBonus;
    }
    bool isVampireBonus() const {
        return m_bVampireBonus;
    }

    bool isCombat() const {
        return m_bCombat;
    }
    void setCombat(bool bCombat = true) {
        m_bCombat = bCombat;
    }

private:
    // Owner of each Relic
    RelicOwner m_RelicOwner[maxRelic];

    // Bonus and penalty values
    int m_SlayerHPModify;
    int m_SlayerToHitModify;
    int m_VampireHPModify;
    int m_VampireToHitModify;

    bool m_bSlayerBonus;
    bool m_bVampireBonus;

    bool m_bCombat;
};

extern CombatInfoManager* g_pCombatInfoManager;

#endif
