//--------------------------------------------------------------------------------
//
// Filename    : LCQueryResultPlayerID.cpp
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

// include files
#include "LCQueryResultPlayerID.h"

#include "WireString.h"


//--------------------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------------------
void LCQueryResultPlayerID::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    //--------------------------------------------------
    // read player id
    //--------------------------------------------------

    de::wire::readString(iStream, m_PlayerID, {1, 20}, "PlayerID");

    //--------------------------------------------------
    // read id existence
    //--------------------------------------------------
    iStream.read(m_bExist);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void LCQueryResultPlayerID::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    //--------------------------------------------------
    // write player id
    //--------------------------------------------------
    de::wire::writeString(oStream, m_PlayerID, {1, 20}, "PlayerID");

    //--------------------------------------------------
    // write id existence
    //--------------------------------------------------
    oStream.write(m_bExist);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string LCQueryResultPlayerID::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "LCQueryResultPlayerID(" << "PlayerID:" << m_PlayerID << ",Exist:" << m_bExist << ")";
    return msg.toString();

    __END_CATCH
}
