//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAddWolf.cc
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCAddWolf.h"

#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCAddWolf::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_ObjectID);

    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    /*
        iStream.read(m_SpriteType);

        iStream.read(m_SubColor);
    */
    iStream.read(m_MainColor);
    iStream.read(m_ItemType);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Dir);
    iStream.read(m_CurrentHP);
    iStream.read(m_MaxHP);
    iStream.read(m_GuildID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCAddWolf::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_ObjectID);

    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    /*
        oStream.write(m_SpriteType);

        oStream.write(m_SubColor);
    */
    oStream.write(m_MainColor);
    oStream.write(m_ItemType);
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Dir);
    oStream.write(m_CurrentHP);
    oStream.write(m_MaxHP);
    oStream.write(m_GuildID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCAddWolf::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCAddWolf(" << "ObjectID:" << m_ObjectID << ",Name:" << m_Name << ",Color:" << m_MainColor
        << ",ItemType:" << (int)m_ItemType << ",X:" << (int)m_X << ",Y:" << (int)m_Y << ",Dir: " << (int)m_Dir
        << ",CurrentHP: " << (int)m_CurrentHP << ",MaxHP: " << (int)m_MaxHP << ",GuildID: " << (int)m_GuildID << ")";

    return msg.toString();

    __END_CATCH
}
