#include "GQuestGiveQuestItemElement.h"

#include "GCSystemMessage.h"
#include "GQuestInventory.h"
#include "Player.h"
#include "PlayerCreature.h"
#include "repository/QuestItemRepository.h"

GQuestElement::ResultType GQuestGiveQuestItemElement::checkCondition(PlayerCreature* pPC) const {
    GQuestInventory& inventory = pPC->getGQuestManager()->getGQuestInventory();
    inventory.getItems().push_back(m_ItemType);
    pPC->getPlayer()->sendPacket(inventory.getInventoryPacket());

    GCSystemMessage gcSM;
    gcSM.setMessage("삿돤훨蛟돛야.");
    pPC->getPlayer()->sendPacket(&gcSM);

    if (m_bSave) {
        defaultQuestItemRepository().insert(pPC->getName(), m_ItemType);
    }

    return OK;
}

GQuestGiveQuestItemElement* GQuestGiveQuestItemElement::makeElement(XMLTree* pTree) {
    GQuestGiveQuestItemElement* pRet = new GQuestGiveQuestItemElement;

    DWORD itemType;
    if (pTree->GetAttribute("id", itemType))
        pRet->m_ItemType = itemType;
    pTree->GetAttribute("save", pRet->m_bSave);

    return pRet;
}

GQuestGiveQuestItemElement g_GiveQuestItemElement;
