#ifndef __COUPLE_MANAGER_H__
#define __COUPLE_MANAGER_H__

#include "Assert.h"
#include "Exception.h"
#include "Types.h"

class PlayerCreature;

// The Sex-to-column mapping ("MalePartnerName" / "FemalePartnerName")
// lives in repository/MySQLCoupleRepository.cpp with the statements that
// use it.

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
