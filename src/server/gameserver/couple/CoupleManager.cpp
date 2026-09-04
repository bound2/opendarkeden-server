#include "CoupleManager.h"

#include "PlayerCreature.h"
#include "repository/CoupleRepository.h"

bool CoupleManager::isCouple(PlayerCreature* pPC1, string name2) {
    __BEGIN_TRY

    Assert(pPC1 != NULL);

    return defaultCoupleRepository().countPairingWithPartner(pPC1->getSex(), pPC1->getName(), name2) >= 1;

    __END_CATCH
}

bool CoupleManager::isCouple(PlayerCreature* pPC1, PlayerCreature* pPC2) {
    __BEGIN_TRY

    Assert(pPC1 != NULL);
    Assert(pPC2 != NULL);

    if (pPC1->getSex() == pPC2->getSex())
        return false;

    return defaultCoupleRepository().countPairing(pPC1->getSex(), pPC1->getName(), pPC2->getSex(), pPC2->getName()) >=
           1;

    __END_CATCH
}

bool CoupleManager::hasCouple(PlayerCreature* pPC) {
    __BEGIN_TRY

    Assert(pPC != NULL);

    return defaultCoupleRepository().countPairingsOf(pPC->getSex(), pPC->getName()) >= 1;

    __END_CATCH
}

bool CoupleManager::getPartnerName(PlayerCreature* pPC, string& partnerName) {
    __BEGIN_TRY

    Assert(pPC != NULL);

    return defaultCoupleRepository().loadPartnerName(pPC->getSex(), pPC->getName(), partnerName);

    __END_CATCH
}

void CoupleManager::makeCouple(PlayerCreature* pPC1, PlayerCreature* pPC2) {
    __BEGIN_TRY

    Assert(pPC1 != NULL);
    Assert(pPC2 != NULL);

    Assert(pPC1->getRace() == pPC2->getRace());
    Assert(pPC1->getSex() != pPC2->getSex());

    defaultCoupleRepository().insertCouple(pPC1->getSex(), pPC1->getName(), pPC2->getSex(), pPC2->getName(),
                                           (uint)pPC1->getRace());

    __END_CATCH
}

void CoupleManager::removeCouple(PlayerCreature* pPC1, PlayerCreature* pPC2) {
    __BEGIN_TRY

    Assert(pPC1 != NULL);
    Assert(pPC2 != NULL);

    Assert(pPC1->getRace() == pPC2->getRace());
    Assert(pPC1->getSex() != pPC2->getSex());

    defaultCoupleRepository().deletePairing(pPC1->getSex(), pPC1->getName(), pPC2->getSex(), pPC2->getName(),
                                            (uint)pPC1->getRace());

    __END_CATCH
}

void CoupleManager::removeCoupleForce(PlayerCreature* pPC1, string strPC2) {
    __BEGIN_TRY

    Assert(pPC1 != NULL);

    defaultCoupleRepository().deletePairingWithPartner(pPC1->getSex(), pPC1->getName(), strPC2,
                                                       (uint)(pPC1->getRace()));

    __END_CATCH
}

void CoupleManager::removeCoupleForce(PlayerCreature* pPC1) {
    __BEGIN_TRY

    Assert(pPC1 != NULL);

    defaultCoupleRepository().deletePairingsOf(pPC1->getSex(), pPC1->getName(), (uint)(pPC1->getRace()));

    __END_CATCH
}

CoupleManager* g_pCoupleManager = NULL;
