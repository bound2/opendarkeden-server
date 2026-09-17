//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMPRecoveryEnd.cpp
// Written By  : Reiot
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCMPRecoveryEnd.h"

//--------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------
GCMPRecoveryEnd::GCMPRecoveryEnd()

{
    __BEGIN_TRY
    m_CurrentMP = 0;
    __END_CATCH
}

//--------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------
GCMPRecoveryEnd::~GCMPRecoveryEnd()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCMPRecoveryEnd::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_CurrentMP);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCMPRecoveryEnd::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_CurrentMP);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCMPRecoveryEnd::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMPRecoveryEnd(" << "CurrentMP:" << (int)m_CurrentMP << ")";
    return msg.toString();

    __END_CATCH
}
