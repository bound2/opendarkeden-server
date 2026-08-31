//////////////////////////////////////////////////////////////////////////////
// Filename    : CGPacketDispatch.cpp
// Description : the gameserver composition root for the CG (client->game)
//               direction (docs/RESTRUCTURING.md task 2.3): every CG
//               packet id is bound to its handler here, instead of each
//               packet class carrying a virtual execute(). Generated
//               from the CG packet list; keep alphabetical.
//////////////////////////////////////////////////////////////////////////////

#include "CGPacketDispatch.h"

#include "CGAbsorbSoul.h"
#include "CGAcceptUnion.h"
#include "CGAddGearToMouse.h"
#include "CGAddInventoryToMouse.h"
#include "CGAddItemToCodeSheet.h"
#include "CGAddItemToItem.h"
#include "CGAddMouseToGear.h"
#include "CGAddMouseToInventory.h"
#include "CGAddMouseToQuickSlot.h"
#include "CGAddMouseToZone.h"
#include "CGAddQuickSlotToMouse.h"
#include "CGAddSMSAddress.h"
#include "CGAddZoneToInventory.h"
#include "CGAddZoneToMouse.h"
#include "CGAppointSubmaster.h"
#include "CGAttack.h"
#include "CGAuthKey.h"
#include "CGBloodDrain.h"
#include "CGBuyStoreItem.h"
#include "CGCastingSkill.h"
#include "CGConnect.h"
#include "CGConnectSetKey.h"
#include "CGCrashReport.h"
#include "CGDeleteSMSAddress.h"
#include "CGDenyUnion.h"
#include "CGDepositPet.h"
#include "CGDialUp.h"
#include "CGDisplayItem.h"
#include "CGDissectionCorpse.h"
#include "CGDonationMoney.h"
#include "CGDownSkill.h"
#include "CGDropMoney.h"
#include "CGExchangeBuy.h"
#include "CGExchangeList.h"
#include "CGExpelGuild.h"
#include "CGExpelGuildMember.h"
#include "CGFailQuest.h"
#include "CGGQuestAccept.h"
#include "CGGQuestCancel.h"
#include "CGGetEventItem.h"
#include "CGGetOffMotorCycle.h"
#include "CGGlobalChat.h"
#include "CGGuildChat.h"
#include "CGJoinGuild.h"
#include "CGLearnSkill.h"
#include "CGLogout.h"
#include "CGLotterySelect.h"
#include "CGMakeItem.h"
#include "CGMixItem.h"
#include "CGModifyGuildIntro.h"
#include "CGModifyGuildMember.h"
#include "CGModifyGuildMemberIntro.h"
#include "CGModifyNickname.h"
#include "CGModifyTaxRatio.h"
#include "CGMouseToStash.h"
#include "CGMove.h"
#include "CGNPCAskAnswer.h"
#include "CGNPCTalk.h"
#include "CGPartyInvite.h"
#include "CGPartyLeave.h"
#include "CGPartyPosition.h"
#include "CGPartySay.h"
#include "CGPetGamble.h"
#include "CGPhoneDisconnect.h"
#include "CGPhoneSay.h"
#include "CGPickupMoney.h"
#include "CGPortCheck.h"
#include "CGQuitGuild.h"
#include "CGQuitUnion.h"
#include "CGQuitUnionAccept.h"
#include "CGQuitUnionDeny.h"
#include "CGRangerSay.h"
#include "CGReady.h"
#include "CGRegistGuild.h"
#include "CGRelicToObject.h"
#include "CGReloadFromInventory.h"
#include "CGReloadFromQuickSlot.h"
#include "CGRequestGuildList.h"
#include "CGRequestGuildMemberList.h"
#include "CGRequestIP.h"
#include "CGRequestInfo.h"
#include "CGRequestNewbieItem.h"
#include "CGRequestPowerPoint.h"
#include "CGRequestRepair.h"
#include "CGRequestStoreInfo.h"
#include "CGRequestUnion.h"
#include "CGRequestUnionInfo.h"
#include "CGResurrect.h"
#include "CGRideMotorCycle.h"
#include "CGSMSAddressList.h"
#include "CGSMSSend.h"
#include "CGSay.h"
#include "CGSelectBloodBible.h"
#include "CGSelectGuild.h"
#include "CGSelectGuildMember.h"
#include "CGSelectNickname.h"
#include "CGSelectPortal.h"
#include "CGSelectQuest.h"
#include "CGSelectRankBonus.h"
#include "CGSelectRegenZone.h"
#include "CGSelectTileEffect.h"
#include "CGSelectWayPoint.h"
#include "CGSetSlayerHotKey.h"
#include "CGSetVampireHotKey.h"
#include "CGShopRequestBuy.h"
#include "CGShopRequestList.h"
#include "CGShopRequestSell.h"
#include "CGSilverCoating.h"
#include "CGSkillToInventory.h"
#include "CGSkillToNamed.h"
#include "CGSkillToObject.h"
#include "CGSkillToSelf.h"
#include "CGSkillToTile.h"
#include "CGStashDeposit.h"
#include "CGStashList.h"
#include "CGStashRequestBuy.h"
#include "CGStashToMouse.h"
#include "CGStashWithdraw.h"
#include "CGStoreClose.h"
#include "CGStoreOpen.h"
#include "CGStoreSign.h"
#include "CGSubmitScore.h"
#include "CGTakeOutGood.h"
#include "CGTameMonster.h"
#include "CGThrowBomb.h"
#include "CGThrowItem.h"
#include "CGTradeAddItem.h"
#include "CGTradeFinish.h"
#include "CGTradeMoney.h"
#include "CGTradePrepare.h"
#include "CGTradeRemoveItem.h"
#include "CGTryJoinGuild.h"
#include "CGTypeStringList.h"
#include "CGUnburrow.h"
#include "CGUndisplayItem.h"
#include "CGUntransform.h"
#include "CGUseBonusPoint.h"
#include "CGUseItemFromGQuestInventory.h"
#include "CGUseItemFromGear.h"
#include "CGUseItemFromInventory.h"
#include "CGUseMessageItemFromInventory.h"
#include "CGUsePotionFromInventory.h"
#include "CGUsePotionFromQuickSlot.h"
#include "CGUsePowerPoint.h"
#include "CGVerifyTime.h"
#include "CGVisible.h"
#include "CGWhisper.h"
#include "CGWithdrawPet.h"
#include "CGWithdrawTax.h"
#include "PacketDispatcher.h"

// Binds one packet class to the static execute() of its handler class,
// preserving the exact call the packet's own execute() used to make.
#define DE_REGISTER_CG(Cls)                                                   \
    {                                                                         \
        struct Thunk {                                                        \
            static void call(Packet* pPacket, Player* pPlayer) {              \
                Cls##Handler::execute(static_cast<Cls*>(pPacket), pPlayer);   \
            }                                                                 \
        };                                                                    \
        PacketDispatcher::registerHandler(Cls().getPacketID(), &Thunk::call); \
    }

namespace {

// CGPortCheckHandler::execute takes no player.
void dispatchCGPortCheck(Packet* pPacket, Player*) {
    CGPortCheckHandler::execute(static_cast<CGPortCheck*>(pPacket));
}

// CGStashList kept a __BEGIN_DEBUG/__END_DEBUG wrapper around its handler.
void dispatchCGStashList(Packet* pPacket, Player* pPlayer) {
    __BEGIN_DEBUG
    CGStashListHandler::execute(static_cast<CGStashList*>(pPacket), pPlayer);
    __END_DEBUG
}

} // namespace

void registerCGPacketHandlers() {
    DE_REGISTER_CG(CGAbsorbSoul);
    DE_REGISTER_CG(CGAcceptUnion);
    DE_REGISTER_CG(CGAddGearToMouse);
    DE_REGISTER_CG(CGAddInventoryToMouse);
    DE_REGISTER_CG(CGAddItemToCodeSheet);
    DE_REGISTER_CG(CGAddItemToItem);
    DE_REGISTER_CG(CGAddMouseToGear);
    DE_REGISTER_CG(CGAddMouseToInventory);
    DE_REGISTER_CG(CGAddMouseToQuickSlot);
    DE_REGISTER_CG(CGAddMouseToZone);
    DE_REGISTER_CG(CGAddQuickSlotToMouse);
    DE_REGISTER_CG(CGAddSMSAddress);
    DE_REGISTER_CG(CGAddZoneToInventory);
    DE_REGISTER_CG(CGAddZoneToMouse);
    DE_REGISTER_CG(CGAppointSubmaster);
    DE_REGISTER_CG(CGAttack);
    DE_REGISTER_CG(CGAuthKey);
    DE_REGISTER_CG(CGBloodDrain);
    DE_REGISTER_CG(CGBuyStoreItem);
    DE_REGISTER_CG(CGCastingSkill);
    DE_REGISTER_CG(CGConnect);
    DE_REGISTER_CG(CGConnectSetKey);
    DE_REGISTER_CG(CGCrashReport);
    DE_REGISTER_CG(CGDeleteSMSAddress);
    DE_REGISTER_CG(CGDenyUnion);
    DE_REGISTER_CG(CGDepositPet);
    DE_REGISTER_CG(CGDialUp);
    DE_REGISTER_CG(CGDisplayItem);
    DE_REGISTER_CG(CGDissectionCorpse);
    DE_REGISTER_CG(CGDonationMoney);
    DE_REGISTER_CG(CGDownSkill);
    DE_REGISTER_CG(CGDropMoney);
    DE_REGISTER_CG(CGExchangeBuy);
    DE_REGISTER_CG(CGExchangeList);
    DE_REGISTER_CG(CGExpelGuild);
    DE_REGISTER_CG(CGExpelGuildMember);
    DE_REGISTER_CG(CGFailQuest);
    DE_REGISTER_CG(CGGQuestAccept);
    DE_REGISTER_CG(CGGQuestCancel);
    DE_REGISTER_CG(CGGetEventItem);
    DE_REGISTER_CG(CGGetOffMotorCycle);
    DE_REGISTER_CG(CGGlobalChat);
    DE_REGISTER_CG(CGGuildChat);
    DE_REGISTER_CG(CGJoinGuild);
    DE_REGISTER_CG(CGLearnSkill);
    DE_REGISTER_CG(CGLogout);
    DE_REGISTER_CG(CGLotterySelect);
    DE_REGISTER_CG(CGMakeItem);
    DE_REGISTER_CG(CGMixItem);
    DE_REGISTER_CG(CGModifyGuildIntro);
    DE_REGISTER_CG(CGModifyGuildMember);
    DE_REGISTER_CG(CGModifyGuildMemberIntro);
    DE_REGISTER_CG(CGModifyNickname);
    DE_REGISTER_CG(CGModifyTaxRatio);
    DE_REGISTER_CG(CGMouseToStash);
    DE_REGISTER_CG(CGMove);
    DE_REGISTER_CG(CGNPCAskAnswer);
    DE_REGISTER_CG(CGNPCTalk);
    DE_REGISTER_CG(CGPartyInvite);
    DE_REGISTER_CG(CGPartyLeave);
    DE_REGISTER_CG(CGPartyPosition);
    DE_REGISTER_CG(CGPartySay);
    DE_REGISTER_CG(CGPetGamble);
    DE_REGISTER_CG(CGPhoneDisconnect);
    DE_REGISTER_CG(CGPhoneSay);
    DE_REGISTER_CG(CGPickupMoney);
    DE_REGISTER_CG(CGQuitGuild);
    DE_REGISTER_CG(CGQuitUnion);
    DE_REGISTER_CG(CGQuitUnionAccept);
    DE_REGISTER_CG(CGQuitUnionDeny);
    DE_REGISTER_CG(CGRangerSay);
    DE_REGISTER_CG(CGReady);
    DE_REGISTER_CG(CGRegistGuild);
    DE_REGISTER_CG(CGRelicToObject);
    DE_REGISTER_CG(CGReloadFromInventory);
    DE_REGISTER_CG(CGReloadFromQuickSlot);
    DE_REGISTER_CG(CGRequestGuildList);
    DE_REGISTER_CG(CGRequestGuildMemberList);
    DE_REGISTER_CG(CGRequestIP);
    DE_REGISTER_CG(CGRequestInfo);
    DE_REGISTER_CG(CGRequestNewbieItem);
    DE_REGISTER_CG(CGRequestPowerPoint);
    DE_REGISTER_CG(CGRequestRepair);
    DE_REGISTER_CG(CGRequestStoreInfo);
    DE_REGISTER_CG(CGRequestUnion);
    DE_REGISTER_CG(CGRequestUnionInfo);
    DE_REGISTER_CG(CGResurrect);
    DE_REGISTER_CG(CGRideMotorCycle);
    DE_REGISTER_CG(CGSMSAddressList);
    DE_REGISTER_CG(CGSMSSend);
    DE_REGISTER_CG(CGSay);
    DE_REGISTER_CG(CGSelectBloodBible);
    DE_REGISTER_CG(CGSelectGuild);
    DE_REGISTER_CG(CGSelectGuildMember);
    DE_REGISTER_CG(CGSelectNickname);
    DE_REGISTER_CG(CGSelectPortal);
    DE_REGISTER_CG(CGSelectQuest);
    DE_REGISTER_CG(CGSelectRankBonus);
    DE_REGISTER_CG(CGSelectRegenZone);
    DE_REGISTER_CG(CGSelectTileEffect);
    DE_REGISTER_CG(CGSelectWayPoint);
    DE_REGISTER_CG(CGSetSlayerHotKey);
    DE_REGISTER_CG(CGSetVampireHotKey);
    DE_REGISTER_CG(CGShopRequestBuy);
    DE_REGISTER_CG(CGShopRequestList);
    DE_REGISTER_CG(CGShopRequestSell);
    DE_REGISTER_CG(CGSilverCoating);
    DE_REGISTER_CG(CGSkillToInventory);
    DE_REGISTER_CG(CGSkillToNamed);
    DE_REGISTER_CG(CGSkillToObject);
    DE_REGISTER_CG(CGSkillToSelf);
    DE_REGISTER_CG(CGSkillToTile);
    DE_REGISTER_CG(CGStashDeposit);
    DE_REGISTER_CG(CGStashRequestBuy);
    DE_REGISTER_CG(CGStashToMouse);
    DE_REGISTER_CG(CGStashWithdraw);
    DE_REGISTER_CG(CGStoreClose);
    DE_REGISTER_CG(CGStoreOpen);
    DE_REGISTER_CG(CGStoreSign);
    DE_REGISTER_CG(CGSubmitScore);
    DE_REGISTER_CG(CGTakeOutGood);
    DE_REGISTER_CG(CGTameMonster);
    DE_REGISTER_CG(CGThrowBomb);
    DE_REGISTER_CG(CGThrowItem);
    DE_REGISTER_CG(CGTradeAddItem);
    DE_REGISTER_CG(CGTradeFinish);
    DE_REGISTER_CG(CGTradeMoney);
    DE_REGISTER_CG(CGTradePrepare);
    DE_REGISTER_CG(CGTradeRemoveItem);
    DE_REGISTER_CG(CGTryJoinGuild);
    DE_REGISTER_CG(CGTypeStringList);
    DE_REGISTER_CG(CGUnburrow);
    DE_REGISTER_CG(CGUndisplayItem);
    DE_REGISTER_CG(CGUntransform);
    DE_REGISTER_CG(CGUseBonusPoint);
    DE_REGISTER_CG(CGUseItemFromGQuestInventory);
    DE_REGISTER_CG(CGUseItemFromGear);
    DE_REGISTER_CG(CGUseItemFromInventory);
    DE_REGISTER_CG(CGUseMessageItemFromInventory);
    DE_REGISTER_CG(CGUsePotionFromInventory);
    DE_REGISTER_CG(CGUsePotionFromQuickSlot);
    DE_REGISTER_CG(CGUsePowerPoint);
    DE_REGISTER_CG(CGVerifyTime);
    DE_REGISTER_CG(CGVisible);
    DE_REGISTER_CG(CGWhisper);
    DE_REGISTER_CG(CGWithdrawPet);
    DE_REGISTER_CG(CGWithdrawTax);

    PacketDispatcher::registerHandler(CGPortCheck().getPacketID(), &dispatchCGPortCheck);
    PacketDispatcher::registerHandler(CGStashList().getPacketID(), &dispatchCGStashList);
}
