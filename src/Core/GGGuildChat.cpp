//////////////////////////////////////////////////////////////////////
//
// Filename    : GGGuildChat.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GGGuildChat.h"


//////////////////////////////////////////////////////////////////////
// Read data from the Datagram object and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GGGuildChat::read(Datagram& iDatagram)

{
    __BEGIN_TRY

    //--------------------------------------------------
    // read command
    //--------------------------------------------------
    iDatagram.read(m_Type);
    iDatagram.read(m_GuildID);

    BYTE szSender;

    iDatagram.read(szSender);

    if (szSender == 0)
        throw InvalidProtocolException("szSender == 0");
    if (szSender > 10)
        throw InvalidProtocolException("too long Sender length");

    iDatagram.read(m_Sender, szSender);
    iDatagram.read(m_Color);

    BYTE szMessage;

    iDatagram.read(szMessage);

    if (szMessage == 0)
        throw InvalidProtocolException("szMessage == 0");
    if (szMessage > 128)
        throw InvalidProtocolException("too long Message length");

    iDatagram.read(m_Message, szMessage);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the Datagram object.
//////////////////////////////////////////////////////////////////////
void GGGuildChat::write(Datagram& oDatagram) const

{
    __BEGIN_TRY

    //--------------------------------------------------
    // write client IP
    //--------------------------------------------------
    BYTE szSender = m_Sender.size();
    BYTE szMessage = m_Message.size();

    if (szSender == 0)
        throw InvalidProtocolException("szSender == 0");
    if (szSender > 10)
        throw InvalidProtocolException("too long Sender length");

    if (szMessage == 0)
        throw InvalidProtocolException("szMessage == 0");
    if (szMessage > 128)
        throw InvalidProtocolException("too long Message length");

    oDatagram.write(m_Type);
    oDatagram.write(m_GuildID);
    oDatagram.write(szSender);
    oDatagram.write(m_Sender);
    oDatagram.write(m_Color);
    oDatagram.write(szMessage);
    oDatagram.write(m_Message);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string GGGuildChat::toString() const

{
    StringStream msg;

    msg << "GGGuildChat(" << "GuildID:" << (int)m_GuildID << ",Sender:" << m_Sender << ",Color:" << (int)m_Color
        << ",Message:" << m_Message << ")";

    return msg.toString();
}
