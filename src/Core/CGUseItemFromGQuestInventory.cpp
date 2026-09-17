//////////////////////////////////////////////////////////////////////////////
// Filename    : CGUseItemFromGQuestInventory.cpp
// Written By  : excel96
// Description :
// When an item in the inventory is used, the client sends X, Y and the ObjectID;
// the server then runs the code that matches the item's class.
//////////////////////////////////////////////////////////////////////////////

#include "CGUseItemFromGQuestInventory.h"

void CGUseItemFromGQuestInventory::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Index);

    __END_CATCH
}

void CGUseItemFromGQuestInventory::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Index);

    __END_CATCH
}

string CGUseItemFromGQuestInventory::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGUseItemFromGQuestInventory(" << ")";
    return msg.toString();

    __END_CATCH
}
