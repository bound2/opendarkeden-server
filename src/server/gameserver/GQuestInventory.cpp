#include "GQuestInventory.h"

#include "repository/QuestItemRepository.h"

void GQuestInventory::load(const string& ownerName) {
    __BEGIN_TRY

    vector<int> itemTypes = defaultQuestItemRepository().loadItemTypes(ownerName);
    for (size_t i = 0; i < itemTypes.size(); i++) {
        getItems().push_back(itemTypes[i]);
    }

    __END_CATCH
}

void GQuestInventory::removeOne(const string& ownerName, ItemType_t item) {
    __BEGIN_TRY

    defaultQuestItemRepository().removeOne(ownerName, item);

    __END_CATCH
}

void GQuestInventory::addOne(ItemType_t item) {
    getItems().push_back(item);
}

void GQuestInventory::saveOne(const string& ownerName, ItemType_t item) {
    addOne(item);

    defaultQuestItemRepository().insert(ownerName, item);
}
