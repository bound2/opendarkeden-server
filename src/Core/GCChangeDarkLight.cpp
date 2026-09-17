//--------------------------------------------------------------------------------
//
// Filename    : GCChangeDarkLight.cpp
// Written By  : Reiot
//
//--------------------------------------------------------------------------------

// include files
#include "GCChangeDarkLight.h"


//--------------------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------------------
void GCChangeDarkLight::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_DarkLevel);
    iStream.read(m_LightLevel);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCChangeDarkLight::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_DarkLevel);
    oStream.write(m_LightLevel);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCChangeDarkLight::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCChangeDarkLight(" << "DarkLevel:" << (int)m_DarkLevel << ",LightLevel:" << (int)m_LightLevel << ")";
    return msg.toString();

    __END_CATCH
}
