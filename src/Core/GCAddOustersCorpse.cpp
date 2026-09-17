//----------------------------------------------------------------------
//
// Filename    : GCAddOustersCorpse.cpp
// Written By  : Reiot
//
//----------------------------------------------------------------------

// include files
#include "GCAddOustersCorpse.h"


//----------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//----------------------------------------------------------------------
void GCAddOustersCorpse::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    m_OustersInfo.read(iStream);
    iStream.read(m_TreasureCount);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCAddOustersCorpse::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    m_OustersInfo.write(oStream);
    oStream.write(m_TreasureCount);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCAddOustersCorpse::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCAddOustersCorpse(" << m_OustersInfo.toString() << ", Count : " << (int)m_TreasureCount << ")";

    return msg.toString();

    __END_CATCH
}
