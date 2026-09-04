#ifndef __COUPLE_MANAGER_H__
#define __COUPLE_MANAGER_H__

#include "Assert.h"
#include "Exception.h"
#include "Types.h"

class PlayerCreature;

// The Sex-to-column mapping this class used to publish moved into
// repository/MySQLCoupleRepository.cpp along with the statements that
// used it: "MalePartnerName" and "FemalePartnerName" are SQL
// identifiers, and a seam whose whole point is to own the SQL should
// own them too rather than leave a second copy here to drift.
// Nothing outside CoupleManager.cpp ever called getFieldName or
// getCounterFieldName.

class CoupleManager {
public:
    CoupleManager() {}

public:
    bool isCouple(PlayerCreature* pPC1, PlayerCreature* pPC2);
    bool hasCouple(PlayerCreature* pPC);
    bool getPartnerName(PlayerCreature* pPC, string& partnerName);
    bool isCouple(PlayerCreature* pPC1, string name2);

public:
    void makeCouple(PlayerCreature* pPC1, PlayerCreature* pPC2);
    void removeCouple(PlayerCreature* pPC1, PlayerCreature* pPC2);
    void removeCoupleForce(PlayerCreature* pPC1, string strPC2);
    void removeCoupleForce(PlayerCreature* pPC1);
};

extern CoupleManager* g_pCoupleManager;

#endif // __COUPLE_MANAGER_H__
