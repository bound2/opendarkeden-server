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

    // 결과 패킷
    GCNPCResponse gcNPCResponse;

    // Dimension ID
    static int dimensionID = g_pConfig->getPropertyInt("Dimension");
    // 월드 ID
    static int worldID = g_pConfig->getPropertyInt("WorldID");

    // affectWorldID
    static int affectWorldID = dimensionID * 3 + worldID;

    // 기부 횟수
    int sumBeforePersonal = 0;
    int sumAfterPersonal = 0;
    int sumBeforeGuild = 0;
    int sumAfterGuild = 0;

    // 기부 이벤트가 활성화된 상태인지 확인한다.
    if (g_pVariableManager->getVariable(DONATION_EVENT_200501) != 1) {
        return;
    }

    // 돈이 충분한지 확인
    if (pPC->getGold() < pPacket->getGold()) {
        gcNPCResponse.setCode(NPC_RESPONSE_NOT_ENOUGH_MONEY);
        pPlayer->sendPacket(&gcNPCResponse);
        return;
    }

    // 인단 돈을 까고
    pPC->decreaseGoldEx(pPacket->getGold());

    // 바뀐 금액을 보낸다.
    GCModifyInformation gcModifyInformation;
    gcModifyInformation.addLongData(MODIFY_GOLD, pPC->getGold());
    pPlayer->sendPacket(&gcModifyInformation);

    // 지금까지의 기부 회수를 구한다.
    sumBeforePersonal = defaultComebackEventRepository().countPersonalDonations(pCreature->getName(), affectWorldID);
    sumBeforeGuild = defaultComebackEventRepository().countGuildDonations(pCreature->getName(), affectWorldID);

    // 기부 내용을 데이터 베이스에 기록한다.
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

    // 지금까지의 기부 회수를 구한다.
    {
        {
            Packet* pNicknamePacket = NULL;

            {
                sumAfterPersonal =
                    defaultComebackEventRepository().countPersonalDonations(pCreature->getName(), affectWorldID);

                // 닉 네임을 추가해야되는 경우
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

                // 닉 네임을 추가해야되는 경우
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

            if (pNicknamePacket != NULL) {
                pGamePlayer->sendPacket(pNicknamePacket);
                SAFE_DELETE(pNicknamePacket);
            }
        }
    }


    // 이펙트를 추가한다.
    if (!pPC->isFlag(Effect::EFFECT_CLASS_DONATION_200501)) {
        EffectDonation200501* pEffect = new EffectDonation200501(pPC);
        pPC->addEffect(pEffect);
        // 강제로 affect 한다. 안에서 브로드캐스팅 등의 처리를 한다.
        pEffect->affect();
    }

    // 기부 결과를 알린다.
    gcNPCResponse.setCode(NPC_RESPONSE_SHOW_DONATION_COMPLETE_DIALOG);
    pPlayer->sendPacket(&gcNPCResponse);

#endif

    __END_CATCH
}
