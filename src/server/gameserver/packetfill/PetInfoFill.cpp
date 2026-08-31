//////////////////////////////////////////////////////////////////////////////
// Filename    : PetInfoFill.cpp
// Description : gameserver-side half of PetInfo — setPetItem() reads the
//               live PetItem, so its definition lives with the game
//               objects, out of the wire library (see src/Core/PetInfo.h).
//////////////////////////////////////////////////////////////////////////////

#include "PetInfo.h"

#include "PetItem.h"

void PetInfo::setPetItem(PetItem* pPetItem) {
    m_pPetItem = pPetItem;
    m_ItemObjectID = (pPetItem == NULL) ? 0 : pPetItem->getObjectID();
}
