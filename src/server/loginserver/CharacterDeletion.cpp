//////////////////////////////////////////////////////////////////////////////
// Filename    : CharacterDeletion.cpp
// Description : the character-deletion decision behind CLDeletePCHandler.
//////////////////////////////////////////////////////////////////////////////

#include "CharacterDeletion.h"

Outcome<void, DeletePCRejection> decideDeletePC(const DeletePCRequest& request,
                                                LoginCharacterPurgeRepository& repository) {
    typedef Outcome<void, DeletePCRejection> Result;

    // The Slayer table indexes every character whatever its race, so the
    // ownership check reads it for all three.
    std::string owner;
    if (!repository.loadActiveSlayerOwner(request.worldID, request.name, owner))
        return Result::Rejected(DeletePCRejection::NoSuchCharacter);

    if (owner != request.playerID)
        return Result::Rejected(DeletePCRejection::NotTheOwner);

    // Retiring the Slayer row is what takes the character out of the
    // account's list; a row in another slot answers false and nothing is
    // purged.
    if (!repository.retireSlayer(request.worldID, request.name, request.slot))
        return Result::Rejected(DeletePCRejection::SlotMismatch);

    return Result::Ok();
}
