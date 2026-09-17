//////////////////////////////////////////////////////////////////////////////
// Filename    : CGUseMessageItemFromInventory.cpp
// Written By  : excel96
// Description :
// When an item in the inventory is used, the client sends X, Y and the ObjectID;
// the server then runs the code that matches the item's class.
//////////////////////////////////////////////////////////////////////////////

#include "CGUseMessageItemFromInventory.h"

#include "Assert1.h"
#include "WireString.h"


void CGUseMessageItemFromInventory::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    CGUseItemFromInventory::read(iStream);


    // message
    de::wire::readString(iStream, m_Message, {1, 128}, "Message");


    __END_CATCH
}

void CGUseMessageItemFromInventory::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    CGUseItemFromInventory::write(oStream);

    // message
    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");


    __END_CATCH
}

string CGUseMessageItemFromInventory::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGUseMessageItemFromInventory(" << "ObjectID:" << (int)getObjectID() << ",InvenX:" << (int)getX()
        << ",InvenY:" << (int)getY() << ",msg:" << m_Message.c_str() << ")";
    return msg.toString();

    __END_CATCH
}
