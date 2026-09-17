//////////////////////////////////////////////////////////////////////
//
// Filename    : GCChangeShape.cpp
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCChangeShape.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCChangeShape::GCChangeShape()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCChangeShape::~GCChangeShape()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCChangeShape::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_ItemClass);
    iStream.read(m_ItemType);
    iStream.read(m_OptionType);
    iStream.read(m_AttackSpeed);
    iStream.read(m_Flag);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCChangeShape::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_ItemClass);
    oStream.write(m_ItemType);
    oStream.write(m_OptionType);
    oStream.write(m_AttackSpeed);
    oStream.write(m_Flag);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// toString()
//////////////////////////////////////////////////////////////////////
string GCChangeShape::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCChangeShape(" << "ObjectID:" << (int)m_ObjectID << ",ItemClass:" << (int)m_ItemClass
        << ",ItemType:" << (int)m_ItemType << ",OptionType:" << (int)m_OptionType
        << ",AttackSpeed :" << (int)m_AttackSpeed << ",Flag :" << (int)m_Flag << ")";
    return msg.toString();

    __END_CATCH
}
