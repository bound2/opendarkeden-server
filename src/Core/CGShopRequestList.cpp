//////////////////////////////////////////////////////////////////////////////
// Filename    : CGShopRequestList.cpp
// Description :
// When the shop version the player holds and the version the server holds
// differ, the player asks the server for the list of goods.
// This is the packet sent at that point.
//////////////////////////////////////////////////////////////////////////////

#include "CGShopRequestList.h"

void CGShopRequestList::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_RackType);

    __END_CATCH
}

void CGShopRequestList::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_RackType);

    __END_CATCH
}

string CGShopRequestList::toString() const {
    StringStream msg;
    msg << "CGShopRequestList(" << "ObjectID:" << (int)m_ObjectID << "RackType:" << (int)m_RackType << ")";
    return msg.toString();
}
