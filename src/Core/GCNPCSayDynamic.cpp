//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNPCSayDynamic.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCNPCSayDynamic.h"

#include "WireString.h"

void GCNPCSayDynamic::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);

    de::wire::readString(iStream, m_Message, {1, kMaxMessageSize}, "Message");

    __END_CATCH
}

void GCNPCSayDynamic::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);

    de::wire::writeString(oStream, m_Message, {1, kMaxMessageSize}, "Message");

    __END_CATCH
}

string GCNPCSayDynamic::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCNPCSayDynamic(" << "ObjectID:" << m_ObjectID << ",Message:" << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
