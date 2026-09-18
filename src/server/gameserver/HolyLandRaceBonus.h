#ifndef __HOLY_LAND_RACE_BONUS_INFO_H__
#define __HOLY_LAND_RACE_BONUS_INFO_H__

#include "Exception.h"
#include "Types.h"

class HolyLandRaceBonus {
public:
    HolyLandRaceBonus();
    ~HolyLandRaceBonus();

    void clear() {
        m_SlayerOptionTypes.clear();
        m_VampireOptionTypes.clear();
    }

    // Recompute the race bonus in Adam's holy land.
    void refresh();

    const list<OptionType_t>& getSlayerOptionTypeList() const {
        return m_SlayerOptionTypes;
    }
    const list<OptionType_t>& getVampireOptionTypeList() const {
        return m_VampireOptionTypes;
    }

public:
    list<OptionType_t> m_SlayerOptionTypes;
    list<OptionType_t> m_VampireOptionTypes;
};

#endif
