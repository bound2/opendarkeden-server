//////////////////////////////////////////////////////////////////////
// Filename    : GCRequestedIP.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////

#include "GCRequestedIP.h"

#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// class GCRequestedIP member methods
//////////////////////////////////////////////////////////////////////

GCRequestedIP::GCRequestedIP()

    {__BEGIN_TRY __END_CATCH}

GCRequestedIP::~GCRequestedIP()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void GCRequestedIP::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_Name, {1, kMaxNameLength}, "Name");

    iStream.read(m_IP);
    iStream.read(m_Port);

    __END_CATCH
}

void GCRequestedIP::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, kMaxNameLength}, "Name");

    oStream.write(m_IP);
    oStream.write(m_Port);

    __END_CATCH
}

string GCRequestedIP::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCRequestedIP(" << "Name: " << m_Name << "IP: " << m_IP << "Port: " << m_Port << ")";
    return msg.toString();

    __END_CATCH
}
