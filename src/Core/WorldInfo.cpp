//////////////////////////////////////////////////////////////////////
//
// Filename    : WorldInfo.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "WorldInfo.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
WorldInfo::WorldInfo() {
    __BEGIN_TRY
    m_Stat = 0;
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
WorldInfo::~WorldInfo() noexcept = default;


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void WorldInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_ID);

    // The name is read unconditionally, so an empty one is refused here
    // although write() emits it.
    de::wire::readString(iStream, m_Name, {1, maxNameLength}, "Name");
    iStream.read(m_Stat);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void WorldInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_ID);
    de::wire::writeString(oStream, m_Name, {0, maxNameLength}, "Name");
    oStream.write(m_Stat);

    __END_CATCH
}

//--------------------------------------------------------------------
// getSize
//--------------------------------------------------------------------
PacketSize_t WorldInfo::getSize() {
    __BEGIN_TRY

    PacketSize_t PacketSize = szWorldID + de::wire::stringWireSize(m_Name) + szBYTE;

    return PacketSize;

    __END_CATCH
}

/////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string WorldInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "WorldInfo( " << "ID : " << m_ID << "Name : " << m_Name << "Stat : " << m_Stat << ")";

    return msg.toString();

    __END_CATCH
}
