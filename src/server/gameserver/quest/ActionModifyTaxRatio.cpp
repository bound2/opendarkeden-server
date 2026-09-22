////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionModifyTaxRatio.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ActionModifyTaxRatio.h"

#include "CastleInfoManager.h"
#include "Creature.h"
#include "GCNPCResponse.h"
#include "GamePlayer.h"
#include "Guild.h"
#include "GuildManager.h"
#include "NPC.h"
#include "PlayerCreature.h"

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ActionModifyTaxRatio::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionModifyTaxRatio::execute(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Assert(pCreature1 != NULL);
    Assert(pCreature2 != NULL);
    Assert(pCreature1->isNPC());
    Assert(pCreature2->isPC());

    bool bSuccess = true;

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature2);

    Player* pPlayer = pPC->getPlayer();
    Assert(pPlayer != NULL);

    GuildID_t guildID = pPC->getGuildID();
    GCNPCResponse deny;

    Guild* pGuild = g_pGuildManager->getGuild(guildID);
    if (bSuccess && pGuild == NULL) {
        // No guild.
        bSuccess = false;
        deny.setCode(NPC_RESPONSE_NO_GUILD);
    }

    if (bSuccess && pGuild->getMaster() != pPC->getName()) {
        // Not the guild master.
        bSuccess = false;
        deny.setCode(NPC_RESPONSE_NOT_GUILD_MASTER);
    }

    // The castles the guild owns. The list is fetched whether or not the
    // checks above already failed.
    list<CastleInfo*> pCastleInfoList = g_pCastleInfoManager->getGuildCastleInfos(guildID);
    if (bSuccess && pCastleInfoList.empty()) {
        // The guild owns no castle.
        bSuccess = false;
        deny.setCode(NPC_RESPONSE_HAS_NO_CASTLE);
    }

    list<CastleInfo*>::iterator itr = pCastleInfoList.begin();
    CastleInfo* pCastleInfo = NULL;

    for (; itr != pCastleInfoList.end(); itr++) {
        if ((*itr)->getZoneID() == pCreature1->getZoneID()) {
            pCastleInfo = (*itr);
            break;
        }
    }

    if (bSuccess && pCastleInfo == NULL) {
        bSuccess = false;
        deny.setCode(NPC_RESPONSE_NOT_YOUR_CASTLE);
    }

    if (bSuccess) {
        GCNPCResponse response;
        response.setCode(NPC_RESPONSE_SHOW_TAX_RATIO);
        response.setParameter((uint)pCastleInfo->getItemTaxRatio());
        pPlayer->sendPacket(&response);
    } else {
        pPlayer->sendPacket(&deny);
    }

    GCNPCResponse quit;
    quit.setCode(NPC_RESPONSE_QUIT_DIALOGUE);
    pPlayer->sendPacket(&quit);

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionModifyTaxRatio::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionModifyTaxRatio(" << ")";

    return msg.toString();

    __END_CATCH
}
