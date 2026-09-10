//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNotifyWin.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCNotifyWin.h"

#include "WireString.h"

void GCNotifyWin::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_GiftID);

    // The factory max budgets more than the length byte can describe, so
    // the byte's own range is the cap.
    de::wire::readString(iStream, m_Name, {1, de::wire::kMaxByteStringLength}, "Name");

    __END_CATCH
}

void GCNotifyWin::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_GiftID);

    de::wire::writeString(oStream, m_Name, {1, de::wire::kMaxByteStringLength}, "Name");

    __END_CATCH
}

string GCNotifyWin::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCNotifyWin(" << "ObjectID:" << m_GiftID << ",Message:" << m_Name << ")";
    return msg.toString();

    __END_CATCH
}
