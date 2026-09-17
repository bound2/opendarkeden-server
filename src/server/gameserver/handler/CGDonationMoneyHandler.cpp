//////////////////////////////////////////////////////////////////////////////
// Filename    : CGDonationMoneyHandler.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGDonationMoney.h"

#ifdef __GAME_SERVER__
#include "Assert.h"
#include "EffectDonation200501.h"
#include "GCModifyInformation.h"
#include "GCNPCResponse.h"
#include "GamePlayer.h"
#include "Guild.h"
#include "GuildManager.h"
#include "NicknameBook.h"
#include "PlayerCreature.h"
#include "Properties.h"
#include "VariableManager.h"
#include "repository/ComebackEventRepository.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGDonationMoneyHandler::execute(CGDonationMoney* pPacket, Player* pPlayer) {
    __BEGIN_TRY

#ifdef __GAME_SERVER__

    Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    Assert(pGamePlayer != NULL);

    Creature* pCreature = pGamePlayer->getCreature();
    Assert(pCreature != NULL);

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Assert(pPC != NULL);

    // Result packet
    GCNPCResponse gcNPCResponse;

    // Dimension ID
    static int dimensionID = g_pConfig->getPropertyInt("Dimension");
    // World ID
    static int worldID = g_pConfig->getPropertyInt("WorldID");

    // affectWorldID
    static int affectWorldID = dimensionID * 3 + worldID;

    // Donation count
    int sumBeforePersonal = 0;
    int sumAfterPersonal = 0;
    int sumBeforeGuild = 0;
    int sumAfterGuild = 0;

    // Check that the donation event is active.
    if (g_pVariableManager->getVariable(DONATION_EVENT_200501) != 1) {
        return;
    }

    // Check that there is enough money
    if (pPC->getGold() < pPacket->getGold()) {
        gcNPCResponse.setCode(NPC_RESPONSE_NOT_ENOUGH_MONEY);
        pPlayer->sendPacket(&gcNPCResponse);
        return;
    }

    // Take the money first
    pPC->decreaseGoldEx(pPacket->getGold());

    // Send the changed amount.
    GCModifyInformation gcModifyInformation;
    gcModifyInformation.addLongData(MODIFY_GOLD, pPC->getGold());
    pPlayer->sendPacket(&gcModifyInformation);

    // Get the donation count so far.
    sumBeforePersonal = defaultComebackEventRepository().countPersonalDonations(pCreature->getName(), affectWorldID);
    sumBeforeGuild = defaultComebackEventRepository().countGuildDonations(pCreature->getName(), affectWorldID);

    // Record the donation in the database.
    {
        if (pPacket->getDonationType() == DONATION_TYPE_200501_PERSONAL) {
            defaultComebackEventRepository().insertPersonalDonation(pGamePlayer->getID(), pCreature->getName(),
                                                                    affectWorldID, pPacket->getGold());
        } else if (pPacket->getDonationType() == DONATION_TYPE_200501_GUILD) {
            defaultComebackEventRepository().insertGuildDonation(pPC->getGuildID(), pPC->getGuildName(),
                                                                 pGamePlayer->getID(), pCreature->getName(),
                                                                 affectWorldID, pPacket->getGold());
        } else {
            return;
        }
    }

    // Get the donation count so far.
    {
        {
            std::unique_ptr<GCNicknameList> pNicknamePacket;

            {
                sumAfterPersonal =
                    defaultComebackEventRepository().countPersonalDonations(pCreature->getName(), affectWorldID);

                // When a nickname has to be added
                if (sumAfterPersonal == 1 && sumBeforePersonal != sumAfterPersonal) {
                    NicknameBook* pNicknameBook = pPC->getNicknameBook();
                    Assert(pNicknameBook != NULL);

                    pNicknameBook->addNewNickname("새해 복 많이 받으세요");
                    pNicknamePacket = pNicknameBook->getNicknameBookListPacket();
                } else if (sumAfterPersonal == 3 && sumBeforePersonal != sumAfterPersonal) {
                    NicknameBook* pNicknameBook = pPC->getNicknameBook();
                    Assert(pNicknameBook != NULL);

                    pNicknameBook->addNewNickname("밝은세상을 위해 노력한");
                    pNicknamePacket = pNicknameBook->getNicknameBookListPacket();
                } else if (sumAfterPersonal == 5 && sumBeforePersonal != sumAfterPersonal) {
                    NicknameBook* pNicknameBook = pPC->getNicknameBook();
                    Assert(pNicknameBook != NULL);

                    pNicknameBook->addNewNickname("진정한 사랑을 아는");
                    pNicknamePacket = pNicknameBook->getNicknameBookListPacket();
                }
            }

            {
                sumAfterGuild =
                    defaultComebackEventRepository().countGuildDonations(pCreature->getName(), affectWorldID);

                // When a nickname has to be added
                if (sumAfterGuild == 1 && sumBeforeGuild != sumAfterGuild) {
                    NicknameBook* pNicknameBook = pPC->getNicknameBook();
                    Assert(pNicknameBook != NULL);

                    pNicknameBook->addNewNickname("따뜻한 세상을 만듭시다");
                    pNicknamePacket = pNicknameBook->getNicknameBookListPacket();
                } else if (sumAfterGuild == 3 && sumBeforeGuild != sumAfterGuild) {
                    NicknameBook* pNicknameBook = pPC->getNicknameBook();
                    Assert(pNicknameBook != NULL);

                    pNicknameBook->addNewNickname("밝은 세상을 만듭시다");
                    pNicknamePacket = pNicknameBook->getNicknameBookListPacket();
                } else if (sumAfterGuild == 5 && sumBeforeGuild != sumAfterGuild) {
                    NicknameBook* pNicknameBook = pPC->getNicknameBook();
                    Assert(pNicknameBook != NULL);

                    pNicknameBook->addNewNickname("사랑을 실천한 사람들");
                    pNicknamePacket = pNicknameBook->getNicknameBookListPacket();
                }
            }

            if (pNicknamePacket != nullptr) {
                pGamePlayer->sendPacket(pNicknamePacket.get());
            }
        }
    }


    // Add the effect.
    if (!pPC->isFlag(Effect::EFFECT_CLASS_DONATION_200501)) {
        EffectDonation200501* pEffect = new EffectDonation200501(pPC);
        pPC->addEffect(pEffect);
        // Force the affect. Broadcasting and the rest happens inside.
        pEffect->affect();
    }

    // Report the donation result.
    gcNPCResponse.setCode(NPC_RESPONSE_SHOW_DONATION_COMPLETE_DIALOG);
    pPlayer->sendPacket(&gcNPCResponse);

#endif

    __END_CATCH
}
