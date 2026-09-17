//----------------------------------------------------------------------
//
// Filename    : GCMorphVampire2.cpp
// Written By  : crazydog
//
//----------------------------------------------------------------------

// include files
#include "GCMorphVampire2.h"


//----------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//----------------------------------------------------------------------
void GCMorphVampire2::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    m_VampireInfo3.read(iStream);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCMorphVampire2::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    m_VampireInfo3.write(oStream);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCMorphVampire2::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMorphVampire2(" << m_VampireInfo3.toString() << ")";
    return msg.toString();

    __END_CATCH
}
