//--------------------------------------------------------------------------------
//
// Filename    : GCSearchMotorcycleOK.cpp
// Description : Packet that tells the player the server-side shop version.
//
//--------------------------------------------------------------------------------

// include files
#include "GCSearchMotorcycleOK.h"

#include "Assert1.h"

//--------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------
void GCSearchMotorcycleOK::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ZoneID);
    iStream.read(m_ZoneX);
    iStream.read(m_ZoneY);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCSearchMotorcycleOK::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ZoneID);
    oStream.write(m_ZoneX);
    oStream.write(m_ZoneY);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCSearchMotorcycleOK::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCSearchMotorcycleOK(" << "ZoneID:" << (int)m_ZoneID << ",ZoneX:" << (int)m_ZoneX
        << ",ZoneY:" << (int)m_ZoneY << ")";
    return msg.toString();

    __END_CATCH
}
