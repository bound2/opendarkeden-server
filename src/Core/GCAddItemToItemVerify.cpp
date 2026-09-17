//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddItemToItemVerify.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCAddItemToItemVerify.h"

//////////////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////////////
void GCAddItemToItemVerify::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Code);

    switch (m_Code) {
    // Codes that need a parameter
    case ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL_DECREASE:
    case ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_MIXING_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_DETACHING_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_REVIVAL_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_UP_GRADE_OK:
        iStream.read(m_Parameter);
        break;
    case ADD_ITEM_TO_ITEM_VERIFY_THREE_ENCHANT_OK:
        iStream.read(m_Parameter);
        iStream.read(m_Parameter2);
        break;
    // Codes that need no parameter
    default:
        break;
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////////////
void GCAddItemToItemVerify::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Code);

    switch (m_Code) {
    // Codes that need a parameter
    case ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL_DECREASE:
    case ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_MIXING_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_DETACHING_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_REVIVAL_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_UP_GRADE_OK:
        oStream.write(m_Parameter);
        break;
    case ADD_ITEM_TO_ITEM_VERIFY_THREE_ENCHANT_OK:
        oStream.write(m_Parameter);
        oStream.write(m_Parameter2);
        break;
    // Codes that need no parameter
    default:
        break;
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Packet size
//////////////////////////////////////////////////////////////////////////////

PacketSize_t GCAddItemToItemVerify::getPacketSize() const

{
    __BEGIN_TRY

    PacketSize_t size = szBYTE;

    switch (m_Code) {
    // Codes that need a parameter
    case ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL_DECREASE:
    case ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_MIXING_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_DETACHING_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_REVIVAL_OK:
    case ADD_ITEM_TO_ITEM_VERIFY_UP_GRADE_OK:
        size += szuint;
        break;
    case ADD_ITEM_TO_ITEM_VERIFY_THREE_ENCHANT_OK:
        size += szuint * 2;
        break;

    // Codes that need no parameter
    default:
        break;
    }

    return size;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCAddItemToItemVerify::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCAddItemToItemVerify(" << "Code : " << (int)m_Code << "Parameter : " << (int)m_Parameter << ")";
    return msg.toString();

    __END_CATCH
}
