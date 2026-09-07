//--------------------------------------------------------------------------------
//
// Filename   : ItemDestroyer.cpp
// Written By : Reiot
//
//--------------------------------------------------------------------------------

// include files
#include "ItemDestroyer.h"

#include "repository/LoginCharacterPurgeRepository.h"

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
ItemDestroyer::ItemDestroyer() {}


//--------------------------------------------------------------------------------
// destroy all item whose owner is ...
//--------------------------------------------------------------------------------
void ItemDestroyer::destroyAll(const string& ownerID) {
    __BEGIN_TRY

    try {
        defaultLoginCharacterPurgeRepository().destroyItems(ownerID);
    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged to
        // DBError.log (its own message dangles); rethrown as the Error the
        // callers of this class expect.
        throw Error("ItemDestroyer::destroyAll : SQL error, see DBError.log");
    }

    __END_CATCH
}

ItemDestroyer* g_pItemDestroyer = NULL;
