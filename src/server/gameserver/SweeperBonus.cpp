//////////////////////////////////////////////////////////////////////////////
// Filename    : SweeperBonus.cpp
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "SweeperBonus.h"

#include "repository/WarInfoRepository.h"

void SweeperBonus::setRace(Race_t race) {
    __BEGIN_TRY

    if (m_Race != race) {
        m_Race = race;
        defaultWarInfoRepository().saveSweeperBonusOwner(m_Race, m_Type);
    }

    __END_CATCH
}

string SweeperBonus::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "SweeperBonus(\n";

    OptionTypeListConstItor itr = m_OptionTypeList.begin();
    for (; itr != m_OptionTypeList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }

    msg << ")\n";

    return msg.toString();

    __END_CATCH
}
