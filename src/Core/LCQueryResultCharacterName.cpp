//--------------------------------------------------------------------------------
//
// Filename    : LCQueryResultCharacterName.cpp
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

// include files
#include "LCQueryResultCharacterName.h"

#include "WireString.h"


//--------------------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------------------
void LCQueryResultCharacterName::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    //--------------------------------------------------
    // read player id
    //--------------------------------------------------

    de::wire::readString(iStream, m_CharacterName, {1, 20}, "CharacterName");

    //--------------------------------------------------
    // read id existence
    //--------------------------------------------------
    iStream.read(m_bExist);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void LCQueryResultCharacterName::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    //--------------------------------------------------
    // write player id
    //--------------------------------------------------
    de::wire::writeString(oStream, m_CharacterName, {1, 20}, "CharacterName");

    //--------------------------------------------------
    // write id existence
    //--------------------------------------------------
    oStream.write(m_bExist);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string LCQueryResultCharacterName::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "LCQueryResultCharacterName(" << "CharacterName:" << m_CharacterName << ",Exist:" << m_bExist << ")";
    return msg.toString();

    __END_CATCH
}
