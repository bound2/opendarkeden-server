# ============================================================================
# de-kernel (docs/RESTRUCTURING.md 2.1) — the wire primitives, built as a
# standalone target whose only include dir is src/Core. Membership is the
# single source of truth in tests/arch/kernel_files.txt (the include-graph
# test binds the same list); a kernel source that reaches for a gameserver
# header fails to compile here, which is the point. Nothing links against
# it yet — the apps still get these objects through Core — but it builds in
# every configuration so the layering rule is enforced by every build.
# ============================================================================
file(STRINGS ${CMAKE_SOURCE_DIR}/tests/arch/kernel_files.txt
     DARKEDEN_KERNEL_ENTRIES)
set(KERNEL_SOURCES "")
foreach(entry ${DARKEDEN_KERNEL_ENTRIES})
    if(entry MATCHES "^src/.*\\.cpp$")
        list(APPEND KERNEL_SOURCES ${CMAKE_SOURCE_DIR}/${entry})
    endif()
endforeach()
add_library(de-kernel STATIC ${KERNEL_SOURCES})
target_include_directories(de-kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(de-kernel PUBLIC Threads::Threads)

# ============================================================================
# Core 库 - 基础核心代码，不依赖服务器类型
# ============================================================================

set(CORE_SOURCES
    SXml.cpp
    WarInfo.cpp
    RaceWarInfo.cpp
    GuildWarInfo.cpp
    LevelWarInfo.cpp
    BloodBibleBonusInfo.cpp
    Assert.cpp
#    Assert1.cpp
    Datagram.cpp
    SerialDatagram.cpp
    DatagramSocket.cpp
    FileAPI.cpp
    GameTime.cpp
    ScriptParameter.cpp
    Player.cpp
    PCSlayerInfo.cpp
    PCVampireInfo.cpp
    PCSlayerInfo2.cpp
    PCVampireInfo2.cpp
    PCSlayerInfo3.cpp
    PCVampireInfo3.cpp
    Properties.cpp
    Resource.cpp
    ResourceManager.cpp
    ServerSocket.cpp
    Socket.cpp
    SocketAPI.cpp
    SocketImpl.cpp
    SocketInputStream.cpp
    SocketOutputStream.cpp
    StringStream.cpp
    Update.cpp
    UpdateManager.cpp
    InventoryInfo.cpp
    InventorySlotInfo.cpp
    RideMotorcycleInfo.cpp
    RideMotorcycleSlotInfo.cpp
    GearSlotInfo.cpp
    GearInfo.cpp
    ExtraInfo.cpp
    ExtraSlotInfo.cpp
    PCItemInfo.cpp
    SubItemInfo.cpp
    ModifyInfo.cpp
    ModifyItemInfo.cpp
    EffectInfo.cpp
    SlayerSkillInfo.cpp
    VampireSkillInfo.cpp
    SubSlayerSkillInfo.cpp
    SubVampireSkillInfo.cpp
    Utility.cpp
    VSDateTime.cpp
    ServerGroupInfo.cpp
    TimeChecker.cpp
    NPCInfo.cpp
    Shape.cpp
    Geometry.cpp
    WorldInfo.cpp
    GuildInfo.cpp
    GuildMemberInfo.cpp
    GuildInfo2.cpp
    GuildMemberInfo2.cpp
    Encrypter.cpp
    PacketDispatcher.cpp
    SocketEncryptInputStream.cpp
    SocketEncryptOutputStream.cpp
    OustersSkillInfo.cpp
    PCOustersInfo.cpp
    PCOustersInfo2.cpp
    PCOustersInfo3.cpp
    SubOustersSkillInfo.cpp
    ItemNameInfo.cpp
    SweeperBonusInfo.cpp
    PetInfo.cpp
    NicknameInfo.cpp
    QuestStatusInfo.cpp
    BloodBibleSignInfo.cpp
    StoreInfo.cpp
    md5.cpp
)

add_library(Core STATIC ${CORE_SOURCES})
# SXml.cpp parses with tinyxml2; PUBLIC so consumers inherit the include path
# and the archive without each having to name it.
target_link_libraries(Core PUBLIC tinyxml2)
target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/item
)

# ============================================================================
# 通用 Packet 源文件定义
# ============================================================================

# GC Packets (Game -> Client)
set(GC_PACKET_SOURCES
    GCActiveGuildList.cpp 
    GCAddBat.cpp 
    GCAddBurrowingCreature.cpp 
    GCAddEffect.cpp 
    GCAddEffectToTile.cpp 
    GCAddGearToInventory.cpp 
    GCAddGearToZone.cpp 
    GCAddHelicopter.cpp 
    GCAddInjuriousCreature.cpp 
    GCAddInstalledMineToZone.cpp 
    GCAddItemToInventory.cpp
    GCAddItemToItemVerify.cpp 
    GCAddItemToZone.cpp
    GCAddMonster.cpp 
    GCAddMonsterCorpse.cpp 
    GCAddMonsterFromBurrowing.cpp 
    GCAddMonsterFromTransformation.cpp 
    GCAddNewItemToZone.cpp 
    GCAddNickname.cpp 
    GCAddNPC.cpp 
    GCAddOusters.cpp 
    GCAddOustersCorpse.cpp 
    GCAddSlayer.cpp 
    GCAddSlayerCorpse.cpp 
    GCAddStoreItem.cpp 
    GCAddVampire.cpp 
    GCAddVampireCorpse.cpp 
    GCAddVampireFromBurrowing.cpp 
    GCAddVampireFromTransformation.cpp 
    GCAddVampirePortal.cpp 
    GCAddWolf.cpp 
    GCAddressListVerify.cpp 
    GCAttack.cpp 
    GCAttackArmsOK1.cpp 
    GCAttackArmsOK2.cpp 
    GCAttackArmsOK3.cpp 
    GCAttackArmsOK4.cpp 
    GCAttackArmsOK5.cpp 
    GCAttackMeleeOK1.cpp 
    GCAttackMeleeOK2.cpp 
    GCAttackMeleeOK3.cpp 
    GCAuthKey.cpp 
    GCBloodBibleList.cpp 
    GCBloodBibleSignInfo.cpp 
    GCBloodBibleStatus.cpp 
    GCBloodDrainOK1.cpp 
    GCBloodDrainOK2.cpp 
    GCBloodDrainOK3.cpp 
    GCCannotAdd.cpp 
    GCCannotUse.cpp 
    GCCastingSkill.cpp 
    GCChangeDarkLight.cpp 
    GCChangeInventoryItemNum.cpp
    GCChangeShape.cpp 
    GCChangeWeather.cpp 
    GCCreateItem.cpp 
    GCCreatureDied.cpp 
    GCCrossCounterOK1.cpp 
    GCCrossCounterOK2.cpp 
    GCCrossCounterOK3.cpp 
    GCDeleteandPickUpOK.cpp 
    GCDeleteEffectFromTile.cpp 
    GCDeleteInventoryItem.cpp 
    GCDeleteObject.cpp 
    GCDisconnect.cpp 
    GCDownSkillFailed.cpp 
    GCDownSkillOK.cpp 
    GCDropItemToZone.cpp 
    GCEnterVampirePortal.cpp 
    GCExecuteElement.cpp 
    GCFakeMove.cpp 
    GCFastMove.cpp 
    GCFlagWarStatus.cpp 
    GCFriendChatting.cpp GCFriendChattingHandler.cpp
    GCGetDamage.cpp 
    GCGetOffMotorCycle.cpp 
    GCGetOffMotorCycleFailed.cpp 
    GCGetOffMotorCycleOK.cpp 
    GCGlobalChat.cpp 
    GCGoodsList.cpp 
    GCGQuestInventory.cpp 
    GCGQuestStatusInfo.cpp 
    GCGQuestStatusModify.cpp 
    GCGuildChat.cpp 
    GCGuildMemberList.cpp 
    GCGuildResponse.cpp 
    GCHolyLandBonusInfo.cpp 
    GCHPRecoveryEndToOthers.cpp 
    GCHPRecoveryEndToSelf.cpp 
    GCHPRecoveryStartToOthers.cpp 
    GCHPRecoveryStartToSelf.cpp 
    GCKickMessage.cpp 
    GCKnockBack.cpp 
    GCKnocksTargetBackOK1.cpp 
    GCKnocksTargetBackOK2.cpp 
    GCKnocksTargetBackOK4.cpp 
    GCKnocksTargetBackOK5.cpp 
    GCLearnSkillFailed.cpp 
    GCLearnSkillOK.cpp 
    GCLearnSkillReady.cpp 
    GCLightning.cpp 
    GCMakeItemFail.cpp 
    GCMakeItemOK.cpp 
    GCMineExplosionOK1.cpp 
    GCMineExplosionOK2.cpp 
    GCMiniGameScores.cpp 
    GCModifyGuildMemberInfo.cpp 
    GCModifyInformation.cpp 
    GCModifyMoney.cpp 
    GCModifyNickname.cpp 
    GCModyfyMoney.cpp 
    GCMonsterKillQuestInfo.cpp 
#    GCMonsterKillQuestStatus.cpp 
    GCMorph1.cpp 
    GCMorphSlayer2.cpp 
    GCMorphVampire2.cpp 
    GCMove.cpp 
    GCMoveError.cpp 
    GCMoveOK.cpp 
    GCMPRecoveryEnd.cpp 
    GCMPRecoveryStart.cpp 
    GCMyStoreInfo.cpp 
    GCNicknameList.cpp 
    GCNicknameVerify.cpp 
    GCNoticeEvent.cpp 
    GCNotifyWin.cpp 
    GCNPCAsk.cpp 
    GCNPCAskDynamic.cpp 
    GCNPCAskVariable.cpp 
    GCNPCInfo.cpp 
    GCNPCResponse.cpp 
    GCNPCSay.cpp 
    GCNPCSayDynamic.cpp 
    GCOtherGuildName.cpp 
    GCOtherModifyInfo.cpp 
    GCOtherStoreInfo.cpp 
    GCPartyError.cpp 
    GCPartyInvite.cpp 
    GCPartyJoined.cpp 
    GCPartyLeave.cpp 
    GCPartyPosition.cpp 
    GCPartySay.cpp 
    GCPetInfo.cpp 
    GCPetStashList.cpp 
    GCPetStashVerify.cpp 
    GCPetUseSkill.cpp 
    GCPhoneConnected.cpp 
    GCPhoneConnectionFailed.cpp 
    GCPhoneDisconnected.cpp 
    GCPhoneSay.cpp 
    GCQuestStatus.cpp 
    GCRankBonusInfo.cpp 
    GCRealWearingInfo.cpp 
    GCReconnect.cpp 
    GCReconnectLogin.cpp 
    GCRegenZoneStatus.cpp 
    GCReloadOK.cpp 
    GCRemoveCorpseHead.cpp 
    GCRemoveEffect.cpp 
    GCRemoveFromGear.cpp 
    GCRemoveInjuriousCreature.cpp 
    GCRemoveStoreItem.cpp 
    GCRequestedIP.cpp 
    GCRequestFailed.cpp 
    GCRequestPowerPointResult.cpp 
    GCRideMotorCycle.cpp 
    GCRideMotorCycleFailed.cpp 
    GCRideMotorCycleOK.cpp 
    GCRing.cpp 
    GCSay.cpp 
    GCSearchMotorcycleFail.cpp 
    GCSearchMotorcycleOK.cpp 
    GCSelectQuestID.cpp 
    GCSelectRankBonusFailed.cpp 
    GCSelectRankBonusOK.cpp 
    GCSetPosition.cpp 
    GCShopBought.cpp 
    GCShopBuyFail.cpp 
    GCShopBuyOK.cpp 
    GCShopList.cpp 
    GCShopListMysterious.cpp 
    GCShopMarketCondition.cpp 
    GCShopSellFail.cpp 
    GCShopSellOK.cpp 
    GCShopSold.cpp 
    GCShopVersion.cpp 
    GCShowGuildInfo.cpp 
    GCShowGuildJoin.cpp 
    GCShowGuildMemberInfo.cpp 
    GCShowGuildRegist.cpp 
    GCShowMessageBox.cpp 
    GCShowUnionInfo.cpp 
    GCShowWaitGuildInfo.cpp 
    GCSkillFailed1.cpp 
    GCSkillFailed2.cpp 
    GCSkillInfo.cpp 
    GCSkillToInventoryOK1.cpp 
    GCSkillToInventoryOK2.cpp 
    GCSkillToObjectOK1.cpp 
    GCSkillToObjectOK2.cpp 
    GCSkillToObjectOK3.cpp 
    GCSkillToObjectOK4.cpp 
    GCSkillToObjectOK5.cpp 
    GCSkillToObjectOK6.cpp 
    GCSkillToSelfOK1.cpp 
    GCSkillToSelfOK2.cpp 
    GCSkillToSelfOK3.cpp 
    GCSkillToTileOK1.cpp 
    GCSkillToTileOK2.cpp 
    GCSkillToTileOK3.cpp 
    GCSkillToTileOK4.cpp 
    GCSkillToTileOK5.cpp 
    GCSkillToTileOK6.cpp 
    GCSMSAddressList.cpp 
    GCStashList.cpp 
    GCStashSell.cpp 
    GCStatusCurrentHP.cpp 
    GCSubInventoryInfo.cpp 
    GCSweeperBonusInfo.cpp 
    GCSystemAvailabilities.cpp 
    GCSystemMessage.cpp 
    GCTakeOff.cpp 
    GCTakeOutFail.cpp 
    GCTakeOutOK.cpp 
    GCTeachSkillInfo.cpp 
    GCThrowBombOK1.cpp 
    GCThrowBombOK2.cpp 
    GCThrowBombOK3.cpp 
    GCThrowItemOK1.cpp 
    GCThrowItemOK2.cpp 
    GCThrowItemOK3.cpp 
    GCTimeLimitItemInfo.cpp 
    GCTradeAddItem.cpp 
    GCTradeError.cpp 
    GCTradeFinish.cpp 
    GCTradeMoney.cpp 
    GCTradePrepare.cpp 
    GCTradeRemoveItem.cpp 
    GCTradeVerify.cpp 
    GCUnburrowFail.cpp 
    GCUnburrowOK.cpp 
    GCUnionOfferList.cpp 
    GCUntransformFail.cpp 
    GCUntransformOK.cpp 
    GCUpdateInfo.cpp 
    GCUseBonusPointFail.cpp 
    GCUseBonusPointOK.cpp 
    GCUseOK.cpp 
    GCUsePowerPointResult.cpp 
    GCVisibleFail.cpp 
    GCVisibleOK.cpp 
    GCWaitGuildList.cpp 
    GCWarList.cpp 
    GCWarScheduleList.cpp 
    GCWhisper.cpp 
    GCWhisperFailed.cpp 
    # Exchange System Packets
    GCExchangeList.cpp 
    GCExchangeBuy.cpp 
)


# CG Packets (Client -> Game)
set(CG_PACKET_SOURCES
    CGAbsorbSoul.cpp CGAbsorbSoulHandler.cpp
    CGAcceptUnion.cpp CGAcceptUnionHandler.cpp
    CGAddGearToMouse.cpp CGAddGearToMouseHandler.cpp
#    CGAddInjuriousCreature.cpp CGAddInjuriousCreatureHandler.cpp
    CGAddInventoryToMouse.cpp CGAddInventoryToMouseHandler.cpp
    CGAddItemToCodeSheet.cpp CGAddItemToCodeSheetHandler.cpp
    CGAddItemToItem.cpp CGAddItemToItemHandler.cpp
    CGAddMouseToGear.cpp CGAddMouseToGearHandler.cpp
    CGAddMouseToInventory.cpp CGAddMouseToInventoryHandler.cpp
    CGAddMouseToQuickSlot.cpp CGAddMouseToQuickSlotHandler.cpp
    CGAddMouseToZone.cpp CGAddMouseToZoneHandler.cpp
    CGAddQuickSlotToMouse.cpp CGAddQuickSlotToMouseHandler.cpp
    CGAddSMSAddress.cpp CGAddSMSAddressHandler.cpp
    CGAddZoneToInventory.cpp CGAddZoneToInventoryHandler.cpp
    CGAddZoneToMouse.cpp CGAddZoneToMouseHandler.cpp
    CGAppointSubmaster.cpp CGAppointSubmasterHandler.cpp
    CGAttack.cpp CGAttackHandler.cpp
    CGAuthKey.cpp CGAuthKeyHandler.cpp
    CGBloodDrain.cpp CGBloodDrainHandler.cpp
    CGBuyStoreItem.cpp CGBuyStoreItemHandler.cpp
    CGCastingSkill.cpp CGCastingSkillHandler.cpp
    CGConnect.cpp CGConnectHandler.cpp
    CGConnectSetKey.cpp CGConnectSetKeyHandler.cpp
    CGCrashReport.cpp CGCrashReportHandler.cpp
    CGDeleteSMSAddress.cpp CGDeleteSMSAddressHandler.cpp
    CGDenyUnion.cpp CGDenyUnionHandler.cpp
    CGDepositPet.cpp CGDepositPetHandler.cpp
    CGDialUp.cpp CGDialUpHandler.cpp
    CGDisplayItem.cpp CGDisplayItemHandler.cpp
    CGDissectionCorpse.cpp CGDissectionCorpseHandler.cpp
    CGDonationMoney.cpp CGDonationMoneyHandler.cpp
    CGDownSkill.cpp CGDownSkillHandler.cpp
    CGDropMoney.cpp CGDropMoneyHandler.cpp
    CGExpelGuild.cpp CGExpelGuildHandler.cpp
    CGExpelGuildMember.cpp CGExpelGuildMemberHandler.cpp
    CGFailQuest.cpp CGFailQuestHandler.cpp
    CGGetEventItem.cpp CGGetEventItemHandler.cpp
    CGGetOffMotorCycle.cpp CGGetOffMotorCycleHandler.cpp
    CGGlobalChat.cpp CGGlobalChatHandler.cpp
    CGGQuestAccept.cpp CGGQuestAcceptHandler.cpp
    CGGQuestCancel.cpp CGGQuestCancelHandler.cpp
    CGGuildChat.cpp CGGuildChatHandler.cpp
    CGJoinGuild.cpp CGJoinGuildHandler.cpp
    CGLearnSkill.cpp CGLearnSkillHandler.cpp
    CGLogout.cpp CGLogoutHandler.cpp
    CGLotterySelect.cpp CGLotterySelectHandler.cpp
    CGMakeItem.cpp CGMakeItemHandler.cpp
    CGMixItem.cpp CGMixItemHandler.cpp
    CGModifyGuildIntro.cpp CGModifyGuildIntroHandler.cpp
    CGModifyGuildMember.cpp CGModifyGuildMemberHandler.cpp
    CGModifyGuildMemberIntro.cpp CGModifyGuildMemberIntroHandler.cpp
    CGModifyNickname.cpp CGModifyNicknameHandler.cpp
    CGModifyTaxRatio.cpp CGModifyTaxRatioHandler.cpp
    CGMouseToStash.cpp CGMouseToStashHandler.cpp
    CGMove.cpp CGMoveHandler.cpp
    CGNPCAskAnswer.cpp CGNPCAskAnswerHandler.cpp
    CGNPCTalk.cpp CGNPCTalkHandler.cpp
    CGPartyInvite.cpp CGPartyInviteHandler.cpp
    CGPartyLeave.cpp CGPartyLeaveHandler.cpp
    CGPartyPosition.cpp CGPartyPositionHandler.cpp
    CGPartySay.cpp CGPartySayHandler.cpp
    CGPetGamble.cpp CGPetGambleHandler.cpp
    CGPhoneDisconnect.cpp CGPhoneDisconnectHandler.cpp
    CGPhoneSay.cpp CGPhoneSayHandler.cpp
    CGPickupMoney.cpp CGPickupMoneyHandler.cpp
    CGPortCheck.cpp CGPortCheckHandler.cpp
    CGQuitGuild.cpp CGQuitGuildHandler.cpp
    CGQuitUnion.cpp CGQuitUnionHandler.cpp
    CGQuitUnionAccept.cpp CGQuitUnionAcceptHandler.cpp
    CGQuitUnionDeny.cpp CGQuitUnionDenyHandler.cpp
    CGRangerSay.cpp CGRangerSayHandler.cpp
    CGReady.cpp CGReadyHandler.cpp
    CGRegistGuild.cpp CGRegistGuildHandler.cpp
    CGRelicToObject.cpp CGRelicToObjectHandler.cpp
    CGReloadFromInventory.cpp CGReloadFromInventoryHandler.cpp
    CGReloadFromQuickSlot.cpp CGReloadFromQuickSlotHandler.cpp
    CGRequestGuildList.cpp CGRequestGuildListHandler.cpp
    CGRequestGuildMemberList.cpp CGRequestGuildMemberListHandler.cpp
    CGRequestInfo.cpp CGRequestInfoHandler.cpp
    CGRequestIP.cpp CGRequestIPHandler.cpp
    CGRequestNewbieItem.cpp CGRequestNewbieItemHandler.cpp
    CGRequestPowerPoint.cpp CGRequestPowerPointHandler.cpp
    CGRequestRepair.cpp CGRequestRepairHandler.cpp
    CGRequestStoreInfo.cpp CGRequestStoreInfoHandler.cpp
    CGRequestUnion.cpp CGRequestUnionHandler.cpp
    CGRequestUnionInfo.cpp CGRequestUnionInfoHandler.cpp
    CGResurrect.cpp CGResurrectHandler.cpp
    CGRideMotorCycle.cpp CGRideMotorCycleHandler.cpp
    CGSay.cpp CGSayHandler.cpp
    CGSelectBloodBible.cpp CGSelectBloodBibleHandler.cpp
    CGSelectGuild.cpp CGSelectGuildHandler.cpp
    CGSelectGuildMember.cpp CGSelectGuildMemberHandler.cpp
    CGSelectNickname.cpp CGSelectNicknameHandler.cpp
    CGSelectPortal.cpp CGSelectPortalHandler.cpp
    CGSelectQuest.cpp CGSelectQuestHandler.cpp
    CGSelectRankBonus.cpp CGSelectRankBonusHandler.cpp
    CGSelectRegenZone.cpp CGSelectRegenZoneHandler.cpp
    CGSelectTileEffect.cpp CGSelectTileEffectHandler.cpp
    CGSelectWayPoint.cpp CGSelectWayPointHandler.cpp
    CGSetSlayerHotKey.cpp CGSetSlayerHotKeyHandler.cpp
    CGSetVampireHotKey.cpp CGSetVampireHotKeyHandler.cpp
    CGShopRequestBuy.cpp CGShopRequestBuyHandler.cpp
    CGShopRequestList.cpp CGShopRequestListHandler.cpp
    CGShopRequestSell.cpp CGShopRequestSellHandler.cpp
    CGSilverCoating.cpp CGSilverCoatingHandler.cpp
    CGSkillToInventory.cpp CGSkillToInventoryHandler.cpp
    CGSkillToNamed.cpp CGSkillToNamedHandler.cpp
    CGSkillToObject.cpp CGSkillToObjectHandler.cpp
    CGSkillToSelf.cpp CGSkillToSelfHandler.cpp
    CGSkillToTile.cpp CGSkillToTileHandler.cpp
    CGSMSAddressList.cpp CGSMSAddressListHandler.cpp
    CGSMSSend.cpp CGSMSSendHandler.cpp
    CGStashDeposit.cpp CGStashDepositHandler.cpp
    CGStashList.cpp CGStashListHandler.cpp
    CGStashRequestBuy.cpp CGStashRequestBuyHandler.cpp
    CGStashToMouse.cpp CGStashToMouseHandler.cpp
    CGStashWithdraw.cpp CGStashWithdrawHandler.cpp
    CGStoreClose.cpp CGStoreCloseHandler.cpp
    CGStoreOpen.cpp CGStoreOpenHandler.cpp
    CGStoreSign.cpp CGStoreSignHandler.cpp
    CGSubmitScore.cpp CGSubmitScoreHandler.cpp
    CGTakeOutGood.cpp CGTakeOutGoodHandler.cpp
    CGTameMonster.cpp CGTameMonsterHandler.cpp
    CGThrowBomb.cpp CGThrowBombHandler.cpp
    CGThrowItem.cpp CGThrowItemHandler.cpp
    CGTradeAddItem.cpp CGTradeAddItemHandler.cpp
    CGTradeFinish.cpp CGTradeFinishHandler.cpp
    CGTradeMoney.cpp CGTradeMoneyHandler.cpp
    CGTradePrepare.cpp CGTradePrepareHandler.cpp
    CGTradeRemoveItem.cpp CGTradeRemoveItemHandler.cpp
    CGTryJoinGuild.cpp CGTryJoinGuildHandler.cpp
    CGTypeStringList.cpp CGTypeStringListHandler.cpp
    CGUnburrow.cpp CGUnburrowHandler.cpp
    CGUndisplayItem.cpp CGUndisplayItemHandler.cpp
    CGUntransform.cpp CGUntransformHandler.cpp
    CGUseBonusPoint.cpp CGUseBonusPointHandler.cpp
    CGUseItemFromGear.cpp CGUseItemFromGearHandler.cpp
    CGUseItemFromGQuestInventory.cpp CGUseItemFromGQuestInventoryHandler.cpp
    CGUseItemFromInventory.cpp CGUseItemFromInventoryHandler.cpp
    CGUseMessageItemFromInventory.cpp CGUseMessageItemFromInventoryHandler.cpp
    CGUsePotionFromInventory.cpp CGUsePotionFromInventoryHandler.cpp
    CGUsePotionFromQuickSlot.cpp CGUsePotionFromQuickSlotHandler.cpp
    CGUsePowerPoint.cpp CGUsePowerPointHandler.cpp
    CGVerifyTime.cpp CGVerifyTimeHandler.cpp
    CGVisible.cpp CGVisibleHandler.cpp
    CGWhisper.cpp CGWhisperHandler.cpp
    CGWithdrawPet.cpp CGWithdrawPetHandler.cpp
    CGWithdrawTax.cpp CGWithdrawTaxHandler.cpp
    # Exchange System Packets
    CGExchangeList.cpp CGExchangeListHandler.cpp
    CGExchangeBuy.cpp CGExchangeBuyHandler.cpp
)

# CL Packets (Client -> Login)
set(CL_PACKET_SOURCES
    CLChangeServer.cpp CLChangeServerHandler.cpp
    CLCreatePC.cpp CLCreatePCHandler.cpp
    CLDeletePC.cpp CLDeletePCHandler.cpp
    CLGetPCList.cpp CLGetPCListHandler.cpp
    CLGetServerList.cpp CLGetServerListHandler.cpp
    CLGetWorldList.cpp CLGetWorldListHandler.cpp
    CLLogin.cpp CLLoginHandler.cpp
    CLLogout.cpp CLLogoutHandler.cpp
    CLQueryCharacterName.cpp CLQueryCharacterNameHandler.cpp
    CLQueryPlayerID.cpp CLQueryPlayerIDHandler.cpp
    CLReconnectLogin.cpp CLReconnectLoginHandler.cpp
    CLRegisterPlayer.cpp CLRegisterPlayerHandler.cpp
    CLSelectPC.cpp CLSelectPCHandler.cpp
    CLSelectServer.cpp CLSelectServerHandler.cpp
    CLSelectWorld.cpp CLSelectWorldHandler.cpp
    CLVersionCheck.cpp CLVersionCheckHandler.cpp
    GCDisconnect.cpp 
	CGConnectSetKey.cpp CGConnectSetKeyHandler.cpp 
)

# LC Packets (Login -> Client)
set(LC_PACKET_SOURCES
    LCCreatePCError.cpp 
    LCCreatePCOK.cpp 
    LCDeletePCError.cpp 
    LCDeletePCOK.cpp 
    LCLoginError.cpp 
    LCLoginOK.cpp 
    LCPCList.cpp 
    LCQueryResultCharacterName.cpp 
    LCQueryResultPlayerID.cpp 
    LCReconnect.cpp 
    LCRegisterPlayerError.cpp 
    LCRegisterPlayerOK.cpp 
    LCSelectPCError.cpp 
    LCServerList.cpp 
    LCVersionCheckError.cpp 
    LCVersionCheckOK.cpp 
    LCWorldList.cpp 
)

# GL Packets (Game -> Login)
set(GL_PACKET_SOURCES
    GLIncomingConnection.cpp GLIncomingConnectionHandler.cpp
    GLIncomingConnectionError.cpp GLIncomingConnectionErrorHandler.cpp
    GLIncomingConnectionOK.cpp GLIncomingConnectionOKHandler.cpp
    GLKickVerify.cpp GLKickVerifyHandler.cpp
    GMServerInfo.cpp GGCommand.cpp GGCommandHandler.cpp
)

# LG Packets (Login -> Game)
set(LG_PACKET_SOURCES
    LGIncomingConnection.cpp LGIncomingConnectionHandler.cpp
    LGIncomingConnectionError.cpp LGIncomingConnectionErrorHandler.cpp
    LGIncomingConnectionOK.cpp LGIncomingConnectionOKHandler.cpp
    LGKickCharacter.cpp LGKickCharacterHandler.cpp
)

# GS Packets (Game -> Shared)
set(GS_PACKET_SOURCES
    GSAddGuild.cpp GSAddGuildHandler.cpp
    GSAddGuildMember.cpp GSAddGuildMemberHandler.cpp
    GSExpelGuildMember.cpp GSExpelGuildMemberHandler.cpp
    GSGuildMemberLogOn.cpp GSGuildMemberLogOnHandler.cpp
    GSModifyGuildIntro.cpp GSModifyGuildIntroHandler.cpp
    GSModifyGuildMember.cpp GSModifyGuildMemberHandler.cpp
    GSQuitGuild.cpp GSQuitGuildHandler.cpp
    GSRequestGuildInfo.cpp GSRequestGuildInfoHandler.cpp
	GGGuildChat.cpp GGGuildChatHandler.cpp
)

# SG Packets (Shared -> Game)
set(SG_PACKET_SOURCES
    SGAddGuildMemberOK.cpp SGAddGuildMemberOKHandler.cpp
    SGAddGuildOK.cpp SGAddGuildOKHandler.cpp
    SGDeleteGuildOK.cpp SGDeleteGuildOKHandler.cpp
    SGExpelGuildMemberOK.cpp SGExpelGuildMemberOKHandler.cpp
    SGGuildInfo.cpp SGGuildInfoHandler.cpp
    SGGuildMemberLogOnOK.cpp SGGuildMemberLogOnOKHandler.cpp
    SGModifyGuildIntroOK.cpp SGModifyGuildIntroOKHandler.cpp
    SGModifyGuildMemberOK.cpp SGModifyGuildMemberOKHandler.cpp
    SGModifyGuildOK.cpp SGModifyGuildOKHandler.cpp
    SGQuitGuildOK.cpp SGQuitGuildOKHandler.cpp
)

# GG Packets (Game -> Game)
set(GG_PACKET_SOURCES
    GGCommand.cpp GGCommandHandler.cpp
    GGGuildChat.cpp GGGuildChatHandler.cpp
    GGServerChat.cpp GGServerChatHandler.cpp
)

# GM Packets
set(GM_PACKET_SOURCES
    GMServerInfo.cpp GMServerInfoHandler.cpp
)

# 通用 Packet 管理
set(PACKET_COMMON_SOURCES
    PacketFactoryManager.cpp
    PacketIDSet.cpp
    PacketValidator.cpp
)

# ============================================================================
# GameServerPackets 库 - 使用 __GAME_SERVER__ 宏编译
# ============================================================================
add_library(GameServerPackets STATIC
    ${GC_PACKET_SOURCES}
    ${CG_PACKET_SOURCES}
    ${GL_PACKET_SOURCES}
    ${LG_PACKET_SOURCES}
    ${GS_PACKET_SOURCES}
    ${SG_PACKET_SOURCES}
    ${GG_PACKET_SOURCES}
    ${GM_PACKET_SOURCES}
    ${PACKET_COMMON_SOURCES}
)
target_compile_definitions(GameServerPackets PRIVATE __GAME_SERVER__ __COMBAT__)
target_include_directories(GameServerPackets PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/database
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/item
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/skill
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/billing
)

# ============================================================================
# LoginServerPackets 库 - 使用 __LOGIN_SERVER__ 宏编译
# ============================================================================
add_library(LoginServerPackets STATIC
    ${CL_PACKET_SOURCES}
    ${LC_PACKET_SOURCES}
    ${GL_PACKET_SOURCES}
    ${LG_PACKET_SOURCES}
    ${GM_PACKET_SOURCES}
    ${PACKET_COMMON_SOURCES}
)
target_compile_definitions(LoginServerPackets PRIVATE __LOGIN_SERVER__)
target_include_directories(LoginServerPackets PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/database
    ${CMAKE_SOURCE_DIR}/src/server/loginserver
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
)

# ============================================================================
# TestPackets - the full packet set with NO server-type macro, for the
# wire-contract tests (tests/). Handlers guard their server logic behind
# #ifdef __GAME_SERVER__ etc., so compiling define-free leaves pure wire
# code with no-op handlers.
#
# NOTE: this is NOT the client's configuration — the client compiles with
# __GAME_CLIENT__=1, so `#ifndef __GAME_CLIENT__` blocks (GCShopList) resolve
# the opposite way here. None of those touch read()/write(), so the goldens
# do reflect what the deployed server emits; but a future
# __GAME_CLIENT__-guarded change to a wire method would be invisible here.
# ============================================================================
if(DARKEDEN_BUILD_TESTS)
    add_library(TestPackets STATIC EXCLUDE_FROM_ALL
        ${GC_PACKET_SOURCES}
        ${CG_PACKET_SOURCES}
        ${CL_PACKET_SOURCES}
        ${LC_PACKET_SOURCES}
        ${GL_PACKET_SOURCES}
        ${LG_PACKET_SOURCES}
        ${GS_PACKET_SOURCES}
        ${SG_PACKET_SOURCES}
        ${GG_PACKET_SOURCES}
        ${GM_PACKET_SOURCES}
        ${PACKET_COMMON_SOURCES}
    )
    # The gameserver include dirs are needed because a handful of packet
    # classes still reach into gameserver headers for constants (e.g.
    # GCAttackArmsOK1 uses SKILL_ATTACK_ARMS from skill/Skill.h) — a Phase 2
    # boundary violation to remove; no server-type macro is defined here.
    target_include_directories(TestPackets PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/server
        ${CMAKE_SOURCE_DIR}/src/server/database
        ${CMAKE_SOURCE_DIR}/src/server/gameserver
        ${CMAKE_SOURCE_DIR}/src/server/gameserver/item
        ${CMAKE_SOURCE_DIR}/src/server/gameserver/skill
        ${CMAKE_SOURCE_DIR}/src/server/gameserver/billing
    )
endif()

# ============================================================================
# SharedServerPackets 库 - 使用 __SHARED_SERVER__ 宏编译
# ============================================================================
add_library(SharedServerPackets STATIC
    ${GS_PACKET_SOURCES}
    ${SG_PACKET_SOURCES}
    ${LG_PACKET_SOURCES}
    ${PACKET_COMMON_SOURCES}
)
target_compile_definitions(SharedServerPackets PRIVATE __SHARED_SERVER__)
target_include_directories(SharedServerPackets PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/database
    ${CMAKE_SOURCE_DIR}/src/server/sharedserver
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
)
