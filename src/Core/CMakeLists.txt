# ============================================================================
# de-kernel (docs/RESTRUCTURING.md 2.1) — the wire primitives, built as a
# standalone target whose only include dir is src/Core. Membership is the
# single source of truth in tests/arch/kernel_files.txt (the include-graph
# test binds the same list); a kernel source that reaches for a gameserver
# header fails to compile here, which is the point. Nothing links against
# it yet — the apps still get these objects through Core — but it builds in
# every configuration so the layering rule is enforced by every build.
# ============================================================================
# REGEX keeps only the entry lines: comment lines can contain ';', which
# CMake list expansion would otherwise split into pseudo-entries. Editing
# the list must retrigger configure, hence CONFIGURE_DEPENDS — without it a
# bare `ninja` keeps building the previous membership.
file(STRINGS ${CMAKE_SOURCE_DIR}/tests/arch/kernel_files.txt
     DARKEDEN_KERNEL_ENTRIES REGEX "^src/")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             ${CMAKE_SOURCE_DIR}/tests/arch/kernel_files.txt)
set(KERNEL_SOURCES "")
foreach(entry ${DARKEDEN_KERNEL_ENTRIES})
    if(entry MATCHES "^src/.*\\.cpp$")
        list(APPEND KERNEL_SOURCES ${CMAKE_SOURCE_DIR}/${entry})
    endif()
endforeach()
add_library(de-kernel STATIC ${KERNEL_SOURCES})
target_include_directories(de-kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(de-kernel PUBLIC Threads::Threads)
# The top-level include_directories() (src/server, src/server/database,
# MySQL) is a directory property this target would inherit, letting a kernel
# source include an app or DB header and compile clean. Pin the target's own
# include path to src/Core alone so the compiler enforces K1 alongside the
# perl checker.
set_target_properties(de-kernel PROPERTIES
    INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}")

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
# The gameserver include dirs Core used to PUBLIC-export are gone: the last
# wire classes that reached into live game objects had those member functions
# split out to src/server/gameserver/packetfill/ (task 2.4), so nothing Core
# compiles needs a gameserver header any more.
target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
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
    GCFriendChatting.cpp 
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
# (GCMonsterKillQuestStatus deleted in 2.4 - dead packet, id enum never existed)
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
    CGAbsorbSoul.cpp 
    CGAcceptUnion.cpp 
    CGAddGearToMouse.cpp 
# (CGAddInjuriousCreature deleted in 2.4 - dead packet, id enum never existed)
    CGAddInventoryToMouse.cpp 
    CGAddItemToCodeSheet.cpp 
    CGAddItemToItem.cpp 
    CGAddMouseToGear.cpp 
    CGAddMouseToInventory.cpp 
    CGAddMouseToQuickSlot.cpp 
    CGAddMouseToZone.cpp 
    CGAddQuickSlotToMouse.cpp 
    CGAddSMSAddress.cpp 
    CGAddZoneToInventory.cpp 
    CGAddZoneToMouse.cpp 
    CGAppointSubmaster.cpp 
    CGAttack.cpp 
    CGAuthKey.cpp 
    CGBloodDrain.cpp 
    CGBuyStoreItem.cpp 
    CGCastingSkill.cpp 
    CGConnect.cpp 
    CGConnectSetKey.cpp 
    CGCrashReport.cpp 
    CGDeleteSMSAddress.cpp 
    CGDenyUnion.cpp 
    CGDepositPet.cpp 
    CGDialUp.cpp 
    CGDisplayItem.cpp 
    CGDissectionCorpse.cpp 
    CGDonationMoney.cpp 
    CGDownSkill.cpp 
    CGDropMoney.cpp 
    CGExpelGuild.cpp 
    CGExpelGuildMember.cpp 
    CGFailQuest.cpp 
    CGGetEventItem.cpp 
    CGGetOffMotorCycle.cpp 
    CGGlobalChat.cpp 
    CGGQuestAccept.cpp 
    CGGQuestCancel.cpp 
    CGGuildChat.cpp 
    CGJoinGuild.cpp 
    CGLearnSkill.cpp 
    CGLogout.cpp 
    CGLotterySelect.cpp 
    CGMakeItem.cpp 
    CGMixItem.cpp 
    CGModifyGuildIntro.cpp 
    CGModifyGuildMember.cpp 
    CGModifyGuildMemberIntro.cpp 
    CGModifyNickname.cpp 
    CGModifyTaxRatio.cpp 
    CGMouseToStash.cpp 
    CGMove.cpp 
    CGNPCAskAnswer.cpp 
    CGNPCTalk.cpp 
    CGPartyInvite.cpp 
    CGPartyLeave.cpp 
    CGPartyPosition.cpp 
    CGPartySay.cpp 
    CGPetGamble.cpp 
    CGPhoneDisconnect.cpp 
    CGPhoneSay.cpp 
    CGPickupMoney.cpp 
    CGPortCheck.cpp 
    CGQuitGuild.cpp 
    CGQuitUnion.cpp 
    CGQuitUnionAccept.cpp 
    CGQuitUnionDeny.cpp 
    CGRangerSay.cpp 
    CGReady.cpp 
    CGRegistGuild.cpp 
    CGRelicToObject.cpp 
    CGReloadFromInventory.cpp 
    CGReloadFromQuickSlot.cpp 
    CGRequestGuildList.cpp 
    CGRequestGuildMemberList.cpp 
    CGRequestInfo.cpp 
    CGRequestIP.cpp 
    CGRequestNewbieItem.cpp 
    CGRequestPowerPoint.cpp 
    CGRequestRepair.cpp 
    CGRequestStoreInfo.cpp 
    CGRequestUnion.cpp 
    CGRequestUnionInfo.cpp 
    CGResurrect.cpp 
    CGRideMotorCycle.cpp 
    CGSay.cpp 
    CGSelectBloodBible.cpp 
    CGSelectGuild.cpp 
    CGSelectGuildMember.cpp 
    CGSelectNickname.cpp 
    CGSelectPortal.cpp 
    CGSelectQuest.cpp 
    CGSelectRankBonus.cpp 
    CGSelectRegenZone.cpp 
    CGSelectTileEffect.cpp 
    CGSelectWayPoint.cpp 
    CGSetSlayerHotKey.cpp 
    CGSetVampireHotKey.cpp 
    CGShopRequestBuy.cpp 
    CGShopRequestList.cpp 
    CGShopRequestSell.cpp 
    CGSilverCoating.cpp 
    CGSkillToInventory.cpp 
    CGSkillToNamed.cpp 
    CGSkillToObject.cpp 
    CGSkillToSelf.cpp 
    CGSkillToTile.cpp 
    CGSMSAddressList.cpp 
    CGSMSSend.cpp 
    CGStashDeposit.cpp 
    CGStashList.cpp 
    CGStashRequestBuy.cpp 
    CGStashToMouse.cpp 
    CGStashWithdraw.cpp 
    CGStoreClose.cpp 
    CGStoreOpen.cpp 
    CGStoreSign.cpp 
    CGSubmitScore.cpp 
    CGTakeOutGood.cpp 
    CGTameMonster.cpp 
    CGThrowBomb.cpp 
    CGThrowItem.cpp 
    CGTradeAddItem.cpp 
    CGTradeFinish.cpp 
    CGTradeMoney.cpp 
    CGTradePrepare.cpp 
    CGTradeRemoveItem.cpp 
    CGTryJoinGuild.cpp 
    CGTypeStringList.cpp 
    CGUnburrow.cpp 
    CGUndisplayItem.cpp 
    CGUntransform.cpp 
    CGUseBonusPoint.cpp 
    CGUseItemFromGear.cpp 
    CGUseItemFromGQuestInventory.cpp 
    CGUseItemFromInventory.cpp 
    CGUseMessageItemFromInventory.cpp 
    CGUsePotionFromInventory.cpp 
    CGUsePotionFromQuickSlot.cpp 
    CGUsePowerPoint.cpp 
    CGVerifyTime.cpp 
    CGVisible.cpp 
    CGWhisper.cpp 
    CGWithdrawPet.cpp 
    CGWithdrawTax.cpp 
    # Exchange System Packets
    CGExchangeList.cpp 
    CGExchangeBuy.cpp 
)

# CL Packets (Client -> Login)
set(CL_PACKET_SOURCES
    CLChangeServer.cpp 
    CLCreatePC.cpp 
    CLDeletePC.cpp 
    CLGetPCList.cpp 
    CLGetServerList.cpp 
    CLGetWorldList.cpp 
    CLLogin.cpp 
    CLLogout.cpp 
    CLQueryCharacterName.cpp 
    CLQueryPlayerID.cpp 
    CLReconnectLogin.cpp 
    CLRegisterPlayer.cpp 
    CLSelectPC.cpp 
    CLSelectServer.cpp 
    CLSelectWorld.cpp 
    CLVersionCheck.cpp 
    GCDisconnect.cpp 
	CGConnectSetKey.cpp 
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
    GLIncomingConnection.cpp 
    GLIncomingConnectionError.cpp 
    GLIncomingConnectionOK.cpp 
    GLKickVerify.cpp 
    GMServerInfo.cpp GGCommand.cpp 
)

# LG Packets (Login -> Game)
set(LG_PACKET_SOURCES
    LGIncomingConnection.cpp 
    LGIncomingConnectionError.cpp 
    LGIncomingConnectionOK.cpp 
    LGKickCharacter.cpp 
)

# GS Packets (Game -> Shared)
set(GS_PACKET_SOURCES
    GSAddGuild.cpp 
    GSAddGuildMember.cpp 
    GSExpelGuildMember.cpp 
    GSGuildMemberLogOn.cpp 
    GSModifyGuildIntro.cpp 
    GSModifyGuildMember.cpp 
    GSQuitGuild.cpp 
    GSRequestGuildInfo.cpp 
	GGGuildChat.cpp 
)

# SG Packets (Shared -> Game)
set(SG_PACKET_SOURCES
    SGAddGuildMemberOK.cpp 
    SGAddGuildOK.cpp 
    SGDeleteGuildOK.cpp 
    SGExpelGuildMemberOK.cpp 
    SGGuildInfo.cpp 
    SGGuildMemberLogOnOK.cpp 
    SGModifyGuildIntroOK.cpp 
    SGModifyGuildMemberOK.cpp 
    SGModifyGuildOK.cpp 
    SGQuitGuildOK.cpp 
)

# GG Packets (Game -> Game)
set(GG_PACKET_SOURCES
    GGCommand.cpp 
    GGGuildChat.cpp 
    GGServerChat.cpp 
)

# GM Packets
set(GM_PACKET_SOURCES
    GMServerInfo.cpp 
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
    # No gameserver include dirs any more: the constants packets used to pull
    # from skill/Skill.h now live in Core (types/SkillTypes.h), and the
    # game-object member functions moved to gameserver/packetfill/ (task 2.4).
    target_include_directories(TestPackets PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/server
        ${CMAKE_SOURCE_DIR}/src/server/database
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
)
