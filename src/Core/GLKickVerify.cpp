//////////////////////////////////////////////////////////////////////
//
// Filename    : GLKickVerify.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GLKickVerify.h"


//////////////////////////////////////////////////////////////////////
// Read data from the Datagram object and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GLKickVerify::read(Datagram& iDatagram)

{
    __BEGIN_TRY

    BYTE bKicked;
    iDatagram.read(bKicked);
    m_bKicked = bKicked;

    iDatagram.read(m_ID);

    //--------------------------------------------------
    // read creature's name
    //--------------------------------------------------
    BYTE szPCName;

    iDatagram.read(szPCName);

    if (szPCName == 0)
        throw InvalidProtocolException("szPCName == 0");

    if (szPCName > 20)
        throw InvalidProtocolException("too long name length");

    iDatagram.read(m_PCName, szPCName);


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the Datagram object.
//////////////////////////////////////////////////////////////////////
void GLKickVerify::write(Datagram& oDatagram) const

{
    __BEGIN_TRY

    BYTE bKicked = (bool)m_bKicked;
    oDatagram.write(bKicked);

    oDatagram.write(m_ID);

    //--------------------------------------------------
    // write PC name
    //--------------------------------------------------
    BYTE szPCName = m_PCName.size();

    if (szPCName == 0)
        throw InvalidProtocolException("szPCName == 0");

    if (szPCName > 20)
        throw InvalidProtocolException("too long name length");

    oDatagram.write(szPCName);

    oDatagram.write(m_PCName);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string GLKickVerify::toString() const

{
    StringStream msg;

    msg << "GLKickVerify(" << "bKicked:" << (int)m_bKicked << ",ID:" << (int)m_ID << ",Name:" << m_PCName << ")";

    return msg.toString();
}
