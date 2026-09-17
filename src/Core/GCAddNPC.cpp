//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAddNPC.cc
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCAddNPC.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCAddNPC::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_ObjectID);

    de::wire::readString(iStream, m_Name, {1, 40}, "Name");
    iStream.read(m_NPCID);

    iStream.read(m_SpriteType);

    iStream.read(m_MainColor);
    iStream.read(m_SubColor);

    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Dir);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCAddNPC::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_ObjectID);

    de::wire::writeString(oStream, m_Name, {1, 40}, "Name");
    oStream.write(m_NPCID);

    oStream.write(m_SpriteType);

    oStream.write(m_MainColor);
    oStream.write(m_SubColor);

    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Dir);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCAddNPC::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCAddNPC(" << "ObjectID:" << m_ObjectID << ",Name:" << m_Name << ",SpriteType:" << (int)m_SpriteType
        << ",MainColor:" << (int)m_MainColor << ",SubColor:" << (int)m_SubColor << ",X:" << (int)m_X
        << ",Y:" << (int)m_Y << ",Dir: " << (int)m_Dir << ")";

    return msg.toString();

    __END_CATCH
}
