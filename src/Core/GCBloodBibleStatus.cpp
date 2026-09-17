//////////////////////////////////////////////////////////////////////
//
// Filename    : GCBloodBibleStatus.cpp
// Written By  : reiot@ewestsoft.com
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCBloodBibleStatus.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCBloodBibleStatus::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ItemType);
    iStream.read(m_ZoneID);
    iStream.read(m_Storage);
    iStream.read(m_Race);
    iStream.read(m_ShrineRace);
    iStream.read(m_X);
    iStream.read(m_Y);

    de::wire::readString(iStream, m_OwnerName, {0, de::wire::kMaxByteStringLength}, "OwnerName");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCBloodBibleStatus::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ItemType);
    oStream.write(m_ZoneID);
    oStream.write(m_Storage);
    oStream.write(m_Race);
    oStream.write(m_ShrineRace);
    oStream.write(m_X);
    oStream.write(m_Y);

    de::wire::writeString(oStream, m_OwnerName, {0, de::wire::kMaxByteStringLength}, "OwnerName");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCBloodBibleStatus::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCBloodBibleStatus(" << "ItemType=" << (int)m_ItemType << ",ZoneID=" << (int)m_ZoneID
        << ",Storage=" << (int)m_Storage << ",OwnerName=" << m_OwnerName.c_str() << ",Race=" << (int)m_Race
        << ",ShrineRace=" << (int)m_ShrineRace << ",X=" << (int)m_X << ",Y=" << (int)m_Y << ")";

    return msg.toString();

    __END_CATCH
}
