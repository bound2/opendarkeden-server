/////////////////////////////////////////////////////////////////////////////
// Filename : PKTErrorHandler.h
// Desc		:
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_ERROR_HANDLER_H__
#define __PKT_ERROR_HANDLER_H__

// include files
#include "MPacketHandler.h"

// class PKTErrorHandler
class PKTErrorHandler : public MPacketHandler {
public:
    // the ID of this packet
    MPacketID_t getID() const;

    // run function
    void execute(MPlayer* pPlayer, MPacket* pPacket);
};

#endif
