root@jhlee-vm-ubuntu:/home/darkeden/vs/src# make -j5
make -C Core
make[1]: Entering directory '/home/darkeden/vs/src/Core'
make[1]: Nothing to be done for 'all'.
make[1]: Leaving directory '/home/darkeden/vs/src/Core'
make -C server
make[1]: Entering directory '/home/darkeden/vs/src/server'
make -C database
make[2]: Entering directory '/home/darkeden/vs/src/server/database'
make[2]: Nothing to be done for 'all'.
make[2]: Leaving directory '/home/darkeden/vs/src/server/database'
make -C chinabilling
make[2]: Entering directory '/home/darkeden/vs/src/server/chinabilling'
make[2]: Nothing to be done for 'all'.
make[2]: Leaving directory '/home/darkeden/vs/src/server/chinabilling'
make -C gameserver
make[2]: Entering directory '/home/darkeden/vs/src/server/gameserver'
make -C mission
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver/mission'
make[3]: Nothing to be done for 'all'.
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver/mission'
make -C couple
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver/couple'
make[3]: Nothing to be done for 'all'.
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver/couple'
make -C war
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver/war'
make[3]: Nothing to be done for 'all'.
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver/war'
make -C item
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver/item'
make[3]: Nothing to be done for 'all'.
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver/item'
make -C skill
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver/skill'
make[3]: '../../../../lib/libSkill.a' is up to date.
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver/skill'
make -C quest
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver/quest'
make -C luaScript 
make[4]: Entering directory '/home/darkeden/vs/src/server/gameserver/quest/luaScript'
make[4]: Nothing to be done for 'all'.
make[4]: Leaving directory '/home/darkeden/vs/src/server/gameserver/quest/luaScript'
make ../../../../lib/libQuest.a
make[4]: Entering directory '/home/darkeden/vs/src/server/gameserver/quest'
make[4]: '../../../../lib/libQuest.a' is up to date.
make[4]: Leaving directory '/home/darkeden/vs/src/server/gameserver/quest'
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver/quest'
make -C billing
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver/billing'
make[3]: Nothing to be done for 'all'.
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver/billing'
make -C ctf
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver/ctf'
make[3]: Nothing to be done for 'all'.
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver/ctf'
make -C mofus
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver/mofus'
make[3]: Nothing to be done for 'all'.
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver/mofus'
make ../../../bin/gameserver
make[3]: Entering directory '/home/darkeden/vs/src/server/gameserver'
g++ -o ../../../bin/gameserver main.o Zone.o Slayer.o Vampire.o Ousters.o OustersEXPInfo.o ClientManager.o ConnectionInfoManager.o Creature.o CreatureManager.o DarkLightInfo.o Corpse.o GamePlayer.o GameServer.o IncomingPlayerManager.o SharedServerClient.o InfoClassManager.o Inventory.o LoginServerManager.o SharedServerManager.o NPC.o NPCManager.o ResurrectLocationManager.o PCFinder.o TelephoneCenter.o ParkingCenter.o PCManager.o Portal.o Script.o ScriptManager.o ThreadManager.o ThreadPool.o Tile.o Sector.o TimeManager.o GoodsInventory.o GoodsInfoManager.o VisionInfo.o WeatherInfo.o WeatherManager.o ZoneInfo.o ZoneGroup.o ZoneGroupManager.o ZoneInfoManager.o ZonePlayerManager.o ZoneGroupThread.o ObjectManager.o AbilityBalance.o PlayerCreature.o ZoneUtil.o Treasure.o PacketUtil.o ConnectionInfo.o InventorySlot.o ObjectRegistry.o WayPoint.o OptionInfo.o VolumeInfo.o VampEXPInfo.o ItemRack.o ShopRack.o Stash.o Garbage.o ShopTemplate.o PriceManager.o CreatureUtil.o ItemMap.o TradeManager.o FlagSet.o AlignmentManager.o SkillInfo.o SkillDomainInfoManager.o SkillParentInfo.o InitAllStat.o PrecedenceTable.o VariableManager.o CombatInfoManager.o UniqueItemManager.o MasterLairInfoManager.o MasterLairManager.o MonsterSummonInfo.o LuckInfo.o LogNameManager.o RankBonusInfo.o RankBonus.o CastleInfoManager.o HolyLandRaceBonus.o ShrineInfoManager.o GlobalItemPositionLoader.o RelicUtil.o HolyLandManager.o BloodBibleBonus.o BloodBibleBonusManager.o CastleShrineInfoManager.o SkillPropertyManager.o StringPool.o PKZoneInfoManager.o GameServerGroupInfoManager.o DefaultOptionSetInfo.o CastleSkillInfo.o TimeLimitItemManager.o ItemMineInfo.o EventItemUtil.o SweeperSet.o SweeperBonus.o SweeperBonusManager.o LevelWar.o LevelWarManager.o LevelWarZoneInfoManager.o RegenZoneManager.o PetTypeInfo.o PetUtil.o PetAttrInfo.o PetExpInfo.o RankExpTable.o SlayerAttrExpTable.o Pet.o SystemAvailabilitiesManager.o LocalIP.o ItemGradeManager.o EventZoneInfo.o SMSServiceThread.o SMSAddressBook.o FiniteStateMachine.o GDRLairManager.o GDRLairAbstractStates.o NicknameBook.o LevelNickInfoManager.o SiegeManager.o BroadcastFilter.o Store.o AdvancementClassExpTable.o DynamicZoneManager.o DynamicZoneInfo.o DynamicZone.o DynamicZoneGroup.o DynamicZoneGateOfAlter.o DynamicZoneAlterOfBlood.o DynamicZoneFactoryManager.o DynamicZoneSlayerMirrorOfAbyss.o DynamicZoneVampireMirrorOfAbyss.o DynamicZoneOustersMirrorOfAbyss.o NewYear2005ItemUtil.o Monster.o MonsterInfo.o MonsterManager.o MonsterAI.o Directive.o MonsterNameManager.o MonsterCounter.o  Item.o ItemInfo.o ItemUtil.o ItemFactoryManager.o ItemInfoManager.o ItemLoaderManager.o SlayerCorpse.o VampireCorpse.o OustersCorpse.o MonsterCorpse.o Party.o Guild.o GuildManager.o GuildUnion.o Effect.o EffectManager.o EffectSchedule.o EffectLoaderManager.o EffectShutDown.o EffectHPRecovery.o EffectMPRecovery.o EffectAlignmentRecovery.o EffectEnemyErase.o EffectDecayCorpse.o EffectDecayItem.o EffectAftermath.o EffectComa.o EffectPrecedence.o EffectTransportItem.o EffectAddItem.o EffectRelicTable.o EffectHasSlayerRelic.o EffectHasVampireRelic.o EffectIncreaseAttr.o EffectDeleteItem.o EffectSlayerRelic.o EffectVampireRelic.o EffectRelicPosition.o EffectRelicLock.o EffectMasterLairPass.o EffectContinualGroundAttack.o EffectGhost.o EffectTransportCreature.o EffectGrandMasterSlayer.o EffectGrandMasterVampire.o EffectKillAftermath.o EffectHasBloodBible.o EffectShrineHoly.o EffectShrineGuard.o EffectShrineShield.o EffectTransportItemToCorpse.o EffectAddItemToCorpse.o EffectHasRelic.o EffectHasCastleSymbol.o EffectLoveChain.o EffectPKZoneRegen.o EffectPKZoneResurrection.o EffectGrandMasterOusters.o EffectTranslation.o EffectLoud.o EffectMute.o SimpleCreatureEffect.o EffectRefiniumTicket.o EffectFlagInsert.o EffectHasSweeper.o EffectKeepSweeper.o EffectRegenZone.o EffectTryingPosition.o EffectTrying.o EffectTryRegenZone.o EffectOnBridge.o EffectHasPet.o EffectPacketSend.o EffectDarknessForbidden.o EffectCastingTrap.o EffectWithWarning.o EffectKickOut.o EffectGDRLairClose.o EffectEventQuestReset.o EffectShareHP.o EffectCanEnterGDRLair.o EffectAutoTurret.o EffectTurretLaser.o EffectKillTimer.o EffectDragonEye.o EffectRegenerate.o EffectRecallMotorcycle.o EffectRideMotorcycle.o EffectDonation200501.o SimpleTileEffect.o EffectDeleteTile.o Event.o EventManager.o EventShutdown.o EventRegeneration.o EventSave.o EventMorph.o EventResurrect.o EventReloadInfo.o EventTransport.o EventKick.o EventSystemMessage.o EventRefreshHolyLandPlayer.o EventHeadCount.o EventCBilling.o EventAuth.o  GQuestInfo.o GQuestManager.o GQuestElement.o GQuestStatus.o GQuestInventory.o GQuestLevelElement.o GQuestTimeElement.o GQuestGiveVampireExpElement.o GQuestBloodDrainElement.o GQuestSayNPCElement.o GQuestExecuteElement.o GQuestGiveQuestItemElement.o GQuestLoseQuestItemElement.o GQuestGiveItemElement.o GQuestTamePetElement.o GQuestRaceElement.o GQuestKilledElement.o GQuestGiveDomainExpElement.o GQuestGiveMoneyElement.o GQuestSkillLevelElement.o GQuestRideMotorcycleElement.o GQuestGiveOustersExpElement.o GQuestTouchWayPointElement.o GQuestHasQuestItemElement.o GQuestPartyDissectElement.o GQuestKillMonsterElement.o GQuestGiveEventQuestItemElement.o GQuestEventPartyElement.o GQuestEventPartyCrashElement.o GQuestCompleteQuestElement.o GQuestFastMoveElement.o GQuestIllegalWarpElement.o GQuestCheckPoint.o GQuestTravelElement.o GQuestORElement.o GQuestNOTElement.o GQuestRandomElement.o GQuestAdvancementClassLevelElement.o GQuestClearDynamicZoneElement.o GQuestAddEffectElement.o GQuestRemoveEffectElement.o GQuestSetEnterDynamicZoneElement.o GQuestEnterDynamicZoneElement.o GQuestStartOtherQuestElement.o GQuestGiveAdvancementClassExpElement.o GQuestAdvanceClassElement.o GQuestWarpElement.o -L../../../lib -lItems -lQuest -lMofus -lGameServerDatabase -lServerCore -lGameServerPackets -lSkill -lCore -lGameServerBilling -lLuaScript -lWar -lCouple -lMission -lCTF -lGameServerCBilling -lpthread -lnsl -lutil -lmysqlclient -llua5.1 -lxerces-c -ldl -lz -rdynamic
/usr/bin/ld: ../../../lib/libGameServerPackets.a(PacketFactoryManager.gs.o): in function `CGExchangeList::CGExchangeList()':
/home/darkeden/vs/src/Core/CGExchangeList.h:20: undefined reference to `vtable for CGExchangeList'
/usr/bin/ld: ../../../lib/libGameServerPackets.a(PacketFactoryManager.gs.o): in function `CGExchangeBuy::CGExchangeBuy()':
/home/darkeden/vs/src/Core/CGExchangeBuy.h:19: undefined reference to `vtable for CGExchangeBuy'
collect2: error: ld returned 1 exit status
make[3]: *** [Makefile:196: ../../../bin/gameserver] Error 1
make[3]: Leaving directory '/home/darkeden/vs/src/server/gameserver'
make[2]: *** [Makefile:177: all] Error 2
make[2]: Leaving directory '/home/darkeden/vs/src/server/gameserver'
make[1]: *** [Makefile:57: all] Error 2
make[1]: Leaving directory '/home/darkeden/vs/src/server'
make: *** [Makefile:22: all] Error 2
root@jhlee-vm-ubuntu:/home/darkeden/vs/src# 