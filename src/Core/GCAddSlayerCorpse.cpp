//--------------------------------------------------------------------------------
//
// Filename    : GCAddSlayerCorpse.cpp
// Written By  : Reiot
//
//--------------------------------------------------------------------------------

// include files
#include "GCAddSlayerCorpse.h"


//--------------------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------------------
void GCAddSlayerCorpse::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    //--------------------------------------------------
    // read slayer info
    //--------------------------------------------------
    m_SlayerInfo.read(iStream);
    iStream.read(m_TreasureCount);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCAddSlayerCorpse::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    //--------------------------------------------------
    // write slayer info
    //--------------------------------------------------
    m_SlayerInfo.write(oStream);
    oStream.write(m_TreasureCount);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCAddSlayerCorpse::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCAddSlayerCorpse(" << m_SlayerInfo.toString() << ", Count : " << (int)m_TreasureCount << ")";

    return msg.toString();

    __END_CATCH
}
