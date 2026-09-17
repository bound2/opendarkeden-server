////////////////////////////////////////////////////////////////////////////////
// Filename    : CGShopRequestSell.cpp
// Description :
// Packet used when a player asks a shop NPC to buy an item the player is
// holding.
////////////////////////////////////////////////////////////////////////////////

#include "CGShopRequestSell.h"

void CGShopRequestSell::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_ItemObjectID);
    iStream.read(m_OpCode);

    __END_CATCH
}

void CGShopRequestSell::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_ItemObjectID);
    oStream.write(m_OpCode);

    __END_CATCH
}

string CGShopRequestSell::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGShopRequestSell(" << "ObjectID : " << (int)m_ObjectID << ")";
    return msg.toString();

    __END_CATCH
}
