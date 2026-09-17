//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNPCResponse.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCNPCResponse.h"

//////////////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////////////
void GCNPCResponse::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Code);

    switch (m_Code) {
    // Codes that need a parameter
    case NPC_RESPONSE_REPAIR_OK:
    case NPC_RESPONSE_SILVER_COATING_OK:
    case NPC_RESPONSE_DONATION_OK:
    case NPC_RESPONSE_DECREASE_BALL:
    case NPC_RESPONSE_GUILD_SHOW_REGIST:
    case NPC_RESPONSE_SHOW_TAX_BALANCE:
    case NPC_RESPONSE_WITHDRAW_TAX_OK: // The guild master succeeded in withdrawing the tax.
    case NPC_RESPONSE_COUPLE_CANNOT_MEET:
    case NPC_RESPONSE_NOT_COUPLE:
    case NPC_RESPONSE_QUEST:
    case NPC_RESPONSE_LOTTERY:
    case NPC_RESPONSE_SHOW_TAX_RATIO:
    case NPC_RESPONSE_SHOW_DONATION_DIALOG:
    case NPC_RESPONSE_SHOW_CONFIRM_GET_EVENT_ITEM_DIALOG:
    case NPC_RESPONSE_SHOW_COMMON_MESSAGE_DIALOG:
        iStream.read(m_Parameter);
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
void GCNPCResponse::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Code);

    switch (m_Code) {
    // Codes that need a parameter
    case NPC_RESPONSE_REPAIR_OK:
    case NPC_RESPONSE_SILVER_COATING_OK:
    case NPC_RESPONSE_DONATION_OK:
    case NPC_RESPONSE_DECREASE_BALL:
    case NPC_RESPONSE_GUILD_SHOW_REGIST:
    case NPC_RESPONSE_SHOW_TAX_BALANCE:
    case NPC_RESPONSE_WITHDRAW_TAX_OK: // The guild master succeeded in withdrawing the tax.
    case NPC_RESPONSE_COUPLE_CANNOT_MEET:
    case NPC_RESPONSE_NOT_COUPLE:
    case NPC_RESPONSE_QUEST:
    case NPC_RESPONSE_LOTTERY:
    case NPC_RESPONSE_SHOW_TAX_RATIO:
    case NPC_RESPONSE_SHOW_DONATION_DIALOG:
    case NPC_RESPONSE_SHOW_CONFIRM_GET_EVENT_ITEM_DIALOG:
    case NPC_RESPONSE_SHOW_COMMON_MESSAGE_DIALOG:
        oStream.write(m_Parameter);
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

PacketSize_t GCNPCResponse::getPacketSize() const

{
    __BEGIN_TRY

    PacketSize_t size = sizeof(m_Code);

    switch (m_Code) {
    // Code to write parameters.
    case NPC_RESPONSE_REPAIR_OK:
    case NPC_RESPONSE_SILVER_COATING_OK:
    case NPC_RESPONSE_DONATION_OK:
    case NPC_RESPONSE_DECREASE_BALL:
    case NPC_RESPONSE_GUILD_SHOW_REGIST:
    case NPC_RESPONSE_SHOW_TAX_BALANCE:
    // The guild master succeeded in finding the tax.
    case NPC_RESPONSE_WITHDRAW_TAX_OK:
    case NPC_RESPONSE_COUPLE_CANNOT_MEET:
    case NPC_RESPONSE_NOT_COUPLE:
    case NPC_RESPONSE_QUEST:
    case NPC_RESPONSE_LOTTERY:
    case NPC_RESPONSE_SHOW_TAX_RATIO:
    case NPC_RESPONSE_SHOW_DONATION_DIALOG:
    case NPC_RESPONSE_SHOW_CONFIRM_GET_EVENT_ITEM_DIALOG:
    case NPC_RESPONSE_SHOW_COMMON_MESSAGE_DIALOG:
        size += szuint;
        break;
    // Code that does not require parameters.
    default:
        break;
    }

    return size;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCNPCResponse::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCNPCResponse(" << "Code : " << (int)m_Code << "Parameter : " << (int)m_Parameter << ")";
    return msg.toString();

    __END_CATCH
}
