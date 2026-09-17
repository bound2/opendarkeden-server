//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddHelicopter.cpp
// Written By  : excel96
// Description :
// Packet sent when a slayer uses a radio to call a helicopter
//////////////////////////////////////////////////////////////////////////////

#include "GCAddHelicopter.h"

//////////////////////////////////////////////////////////////////////////////
// class GCAddHelicopter member methods
//////////////////////////////////////////////////////////////////////////////

void GCAddHelicopter::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_Code);

    __END_CATCH
}

void GCAddHelicopter::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_Code);

    __END_CATCH
}

string GCAddHelicopter::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCAddHelicopter(" << "ObjectID:" << (int)m_ObjectID << ",Code:" << (int)m_Code << ")";
    return msg.toString();

    __END_CATCH
}
