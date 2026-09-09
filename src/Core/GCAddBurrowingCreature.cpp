//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddBurrowingCreature.cc
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCAddBurrowingCreature.h"

#include "WireString.h"

void GCAddBurrowingCreature::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);

    de::wire::readString(iStream, m_Name, {1, 20}, "Name");


    iStream.read(m_X);
    iStream.read(m_Y);
    __END_CATCH
}

void GCAddBurrowingCreature::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);

    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");

    oStream.write(m_X);
    oStream.write(m_Y);
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCAddBurrowingCreature::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCAddBurrowingCreature(" << "ObjectID:" << m_ObjectID << ",Name:" << m_Name << ",X:" << (int)m_X
        << ",Y:" << (int)m_Y << ")";
    return msg.toString();

    __END_CATCH
}
