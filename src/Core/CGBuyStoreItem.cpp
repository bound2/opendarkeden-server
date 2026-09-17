////////////////////////////////////////////////////////////////////////////////
// Filename    : CGBuyStoreItem.cpp
// Description :
// Sent when a player looks at a shop NPC's display window and wants to buy
// an item. The server checks that the player has enough money and enough
// room in the inventory, then hands the item over to the player.
////////////////////////////////////////////////////////////////////////////////

#include "CGBuyStoreItem.h"

void CGBuyStoreItem::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_OwnerObjectID);
    iStream.read(m_ItemObjectID);
    iStream.read(m_Index);

    __END_CATCH
}

void CGBuyStoreItem::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_OwnerObjectID);
    oStream.write(m_ItemObjectID);
    oStream.write(m_Index);

    __END_CATCH
}

string CGBuyStoreItem::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGBuyStoreItem(" << ")";
    return msg.toString();

    __END_CATCH
}
