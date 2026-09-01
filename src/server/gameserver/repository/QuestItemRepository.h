#ifndef __QUEST_ITEM_REPOSITORY_H__
#define __QUEST_ITEM_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the GQuestItemObject table (task 3.2): the
// per-character bag of quest items — one row per item instance (an
// auto-increment ItemID the server never reads, an ItemType, the
// OwnerID). Written by GQuestInventory and by the two quest elements
// that hand items out (GQuestGiveQuestItemElement,
// GQuestGiveEventQuestItemElement); purged with the character in
// CreatureUtil.cpp / CLDeletePCHandler (not enclosed here).
class QuestItemRepository {
public:
    virtual ~QuestItemRepository() {}

    // Every item type the owner holds, one entry per row (duplicates
    // are distinct items), as the driver's getInt returned them.
    virtual std::vector<int> loadItemTypes(const std::string& ownerName) = 0;

    // One more item of that type.
    virtual void insert(const std::string& ownerName, ItemType_t itemType) = 0;

    // Removes ONE row of that type (LIMIT 1) — the other instances of
    // the same item type stay.
    virtual void removeOne(const std::string& ownerName, ItemType_t itemType) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLQuestItemRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
QuestItemRepository& defaultQuestItemRepository();

#endif
