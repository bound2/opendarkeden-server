//--------------------------------------------------------------------------------
//
// Filename    : UCUpdateHandler.cpp
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

// include files
#include "UCUpdate.h"

#ifdef __UPDATE_CLIENT__

/*
    #include <errno.h>
    #include <fcntl.h>
    #include <stdio.h>
    #include <unistd.h>

    #include <sys/types.h>
*/

#include "Assert.h"
#include "Properties.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Upackets/CUEndUpdate.h"
#include "Upackets/CURequest.h"
#include "UpdateManager.h"
#endif

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void UCUpdateHandler::execute(UCUpdate* pPacket, Player* pPlayer) throw(ProtocolException, Error) {
    __BEGIN_TRY
    __END_CATCH
}
