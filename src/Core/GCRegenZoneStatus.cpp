//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRegenZoneStatus.cpp
// Written By  : reiot@ewestsoft.com
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCRegenZoneStatus.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCRegenZoneStatus::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The statuses replace the ones the packet holds.
    for (uint i = 0; i < kZoneCount; ++i)
        iStream.read(m_Statuses[i]);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCRegenZoneStatus::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    for (uint i = 0; i < kZoneCount; ++i)
        oStream.write(m_Statuses[i]);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCRegenZoneStatus::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCRegenZoneStatus(" << ")";

    return msg.toString();

    __END_CATCH
}
