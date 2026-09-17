//////////////////////////////////////////////////////////////////////////////
// Filename    : CGWithdrawTaxHandler.cp	p
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGWithdrawTax.h"

#ifdef __GAME_SERVER__
#include "CastleInfoManager.h"
#include "GCModifyInformation.h"
#include "GCNPCResponse.h"
#include "GamePlayer.h"
#include "GuildManager.h"
#include "PlayerCreature.h"
#endif // __GAME_SERVER__

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGWithdrawTaxHandler::execute(CGWithdrawTax* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    Assert(pGamePlayer != NULL);

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pGamePlayer->getCreature());
    Assert(pPC != NULL);

    GuildID_t guildID = pPC->getGuildID();
    Gold_t gold = pPacket->getGold();

    list<CastleInfo*> pCastleInfoList = g_pCastleInfoManager->getGuildCastleInfos(guildID);
    if (pCastleInfoList.empty()) {
        GCNPCResponse fail;
        fail.setCode(NPC_RESPONSE_WITHDRAW_TAX_FAIL);

        pGamePlayer->sendPacket(&fail);
        return;
    }

    bool bOwner = false;
    list<CastleInfo*>::iterator itr = pCastleInfoList.begin();
    CastleInfo* pCastleInfo = NULL;
    for (; itr != pCastleInfoList.end(); itr++) {
        if ((*itr)->getZoneID() == pPC->getZoneID()) {
            pCastleInfo = (*itr);
            bOwner = true;
            break;
        }
    }

    if (!g_pGuildManager->isGuildMaster(guildID, pPC) // not the guild master.
        || !bOwner                                    // not a castle this player's guild has taken.
        || gold == 0                                  // is this a joke?
        || pCastleInfo->getTaxBalance() < gold        // not enough money.
    ) {
        GCNPCResponse fail;
        fail.setCode(NPC_RESPONSE_WITHDRAW_TAX_FAIL);

        pGamePlayer->sendPacket(&fail);
        return;
    }

    // Every condition is met. Now withdraw the money and give it to the player.
    Gold_t remainBalance = pCastleInfo->decreaseTaxBalanceEx(gold);
    pPC->increaseGoldEx(gold);

    // Send the information that the user's money grew.
    GCModifyInformation gcMI;
    gcMI.addLongData(MODIFY_GOLD, pPC->getGold());

    pGamePlayer->sendPacket(&gcMI);

    // Report that the withdrawal succeeded.
    GCNPCResponse success;
    success.setCode(NPC_RESPONSE_WITHDRAW_TAX_OK);
    success.setParameter(remainBalance);

    pGamePlayer->sendPacket(&success);

#endif // __GAME_SERVER__

    __END_DEBUG_EX __END_CATCH
}
