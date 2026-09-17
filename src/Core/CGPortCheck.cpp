//////////////////////////////////////////////////////////////////////
//
// Filename    : CGPortCheck.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "CGPortCheck.h"


//////////////////////////////////////////////////////////////////////
// Read data from the Datagram object and initialise the packet.
//////////////////////////////////////////////////////////////////////
void CGPortCheck::read(Datagram& iDatagram)

{
    __BEGIN_TRY

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
void CGPortCheck::write(Datagram& oDatagram) const

{
    __BEGIN_TRY

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
string CGPortCheck::toString() const

{
    StringStream msg;

    msg << "CGPortCheck(" << ",PCName:" << m_PCName << ")";

    return msg.toString();
}
