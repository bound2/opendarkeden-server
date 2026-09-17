//////////////////////////////////////////////////////////////////////////////
// Filename    : InventorySlotInfo.cpp
// Written By  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "InventorySlotInfo.h"

//////////////////////////////////////////////////////////////////////////////
// read data from socket input stream
//////////////////////////////////////////////////////////////////////////////
void InventorySlotInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    // First read in the base class.
    PCItemInfo::read(iStream);

    // Read the data belonging to this class.
    iStream.read(m_InvenX);
    iStream.read(m_InvenY);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// write data to socket output stream
//////////////////////////////////////////////////////////////////////////////
void InventorySlotInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // First write in the base class.
    PCItemInfo::write(oStream);

    // Write the data belonging to this class.
    oStream.write(m_InvenX);
    oStream.write(m_InvenY);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////////////
string InventorySlotInfo::toString() const {
    StringStream msg;

    msg << "InventorySlotInfo(" << PCItemInfo::toString() << ",InvenX:" << (int)m_InvenX << ",InvenY:" << (int)m_InvenY
        << ")";

    return msg.toString();
}
