//////////////////////////////////////////////////////////////////////
//
// Filename    : GCHPRecoveryEndToSelf.cpp
// Written By  : Reiot
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCHPRecoveryEndToSelf.h"

//--------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------
GCHPRecoveryEndToSelf::GCHPRecoveryEndToSelf()

{
    __BEGIN_TRY
    m_CurrentHP = 0;
    __END_CATCH
}

//--------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------
GCHPRecoveryEndToSelf::~GCHPRecoveryEndToSelf()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCHPRecoveryEndToSelf::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_CurrentHP);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCHPRecoveryEndToSelf::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_CurrentHP);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCHPRecoveryEndToSelf::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCHPRecoveryEndToSelf(" << ",CurrentHP:" << (int)m_CurrentHP << ")";
    return msg.toString();

    __END_CATCH
}
