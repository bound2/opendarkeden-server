//----------------------------------------------------------------------
//
// Filename    : GCMorphSlayer2.cpp
// Written By  : crazydog
//
//----------------------------------------------------------------------

// include files
#include "GCMorphSlayer2.h"


//----------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//----------------------------------------------------------------------
void GCMorphSlayer2::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    m_SlayerInfo3.read(iStream);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCMorphSlayer2::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    m_SlayerInfo3.write(oStream);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCMorphSlayer2::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMorphSlayer2(" << m_SlayerInfo3.toString() << ")";
    return msg.toString();

    __END_CATCH
}
