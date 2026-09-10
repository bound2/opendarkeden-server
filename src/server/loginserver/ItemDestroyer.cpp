//--------------------------------------------------------------------------------
//
// Filename   : ItemDestroyer.cpp
// Written By : Reiot
//
//--------------------------------------------------------------------------------

// include files
#include "ItemDestroyer.h"

#include "DatabaseError.h"
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
    } catch (const DatabaseError& error) {
        // A SQL failure arrives as END_DB's DatabaseError carrying the line
        // it wrote to DBError.log; rethrown as the Error the callers of this
        // class expect, with that line in it.
        throw Error("ItemDestroyer::destroyAll : " + error.message());
    }

    __END_CATCH
}

ItemDestroyer* g_pItemDestroyer = NULL;
