//////////////////////////////////////////////////////////////////////////////
// Filename    : PetInfoFill.cpp
// Description : gameserver-side half of PetInfo — setPetItem() installs the
//               live PetItem plus the thunk that reads its ObjectID, so the
//               game-object definition lives here, out of the wire library
//               (see src/Core/PetInfo.h).
//////////////////////////////////////////////////////////////////////////////

#include "PetInfo.h"
#include "PetItem.h"

namespace {
// Read the ObjectID at call time, never earlier: ObjectIDs are zone-local
// and reassigned on every zone entry, and a freshly created item carries
// none until registerObject() runs (Object::getObjectID() asserts on it).
// PetInfo::write() therefore resolves the id through this thunk at the
// moment the bytes go out, exactly like the pre-split inline lookup did.
ObjectID_t livePetItemObjectID(const PetItem* pPetItem) {
    return pPetItem->getObjectID();
}
} // namespace

void PetInfo::setPetItem(PetItem* pPetItem) {
    m_pPetItem = pPetItem;
    m_GetItemObjectID = (pPetItem == NULL) ? NULL : &livePetItemObjectID;
}
