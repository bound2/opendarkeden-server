//----------------------------------------------------------------------
//
// Filename    : GCAddVampireCorpse.cpp
// Written By  : Reiot
//
//----------------------------------------------------------------------

// include files
#include "GCAddVampireCorpse.h"


//----------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//----------------------------------------------------------------------
void GCAddVampireCorpse::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    m_VampireInfo.read(iStream);
    iStream.read(m_TreasureCount);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCAddVampireCorpse::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    m_VampireInfo.write(oStream);
    oStream.write(m_TreasureCount);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCAddVampireCorpse::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCAddVampireCorpse(" << m_VampireInfo.toString() << ", Count : " << (int)m_TreasureCount << ")";

    return msg.toString();

    __END_CATCH
}
