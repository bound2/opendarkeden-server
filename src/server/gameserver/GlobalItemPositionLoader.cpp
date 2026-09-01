#include "GlobalItemPositionLoader.h"

#include "CorpseItemPosition.h"
#include "InventoryItemPosition.h"
#include "MouseItemPosition.h"
#include "ZoneItemPosition.h"
#include "repository/ItemRepository.h"

GlobalItemPosition* GlobalItemPositionLoader::load(Item::ItemClass itemClass, ItemID_t itemID)

{
    __BEGIN_TRY

    GlobalItemPosition* pRet = NULL;

    ItemPositionRow row;
    if (defaultItemRepository().loadItemPosition(ItemObjectTableName[(int)itemClass], itemID, row)) {
        GlobalDBItemPosition gip;
        gip.OwnerID = row.ownerID;
        gip.ItemStorage = (Storage)row.storage;
        gip.StorageID = (StorageID_t)row.storageID;
        gip.X = row.x;
        gip.Y = row.y;
        gip.ObjectID = row.objectID;

        pRet = makeGlobalItemPosition(gip);
    } else {
        pRet = NULL;
    }

    return pRet;

    __END_CATCH
}

GlobalItemPosition* GlobalItemPositionLoader::makeGlobalItemPosition(GlobalDBItemPosition& gip)

{
    __BEGIN_TRY

    switch (gip.ItemStorage) {
    case STORAGE_INVENTORY: {
        InventoryItemPosition* pIIP = new InventoryItemPosition();
        pIIP->setOwnerName(gip.OwnerID);
        pIIP->setInventoryX(gip.X);
        pIIP->setInventoryY(gip.Y);
        return (GlobalItemPosition*)pIIP;
    } break;
    case STORAGE_EXTRASLOT: {
        MouseItemPosition* pMIP = new MouseItemPosition();
        pMIP->setOwnerName(gip.OwnerID);
        return (GlobalItemPosition*)pMIP;
    } break;
    case STORAGE_ZONE: {
        ZoneItemPosition* pZIP = new ZoneItemPosition();
        pZIP->setZoneID(gip.StorageID);
        pZIP->setZoneX(gip.X);
        pZIP->setZoneY(gip.Y);
        return (GlobalItemPosition*)pZIP;
    } break;
    case STORAGE_CORPSE: {
        CorpseItemPosition* pCIP = new CorpseItemPosition();
        pCIP->setZoneID(atoi(gip.OwnerID.c_str()));
        pCIP->setCorpseObjectID(gip.StorageID);
        pCIP->setObjectID(gip.ObjectID);

        return (GlobalItemPosition*)pCIP;
    } break;

    default:
        return NULL;
        break;
    }

    __END_CATCH
}
