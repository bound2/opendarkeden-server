//////////////////////////////////////////////////////////////////////////////
// Filename    : CLVersionCheck.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLVersionCheck.h"

void CLVersionCheck::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Version);

    __END_CATCH
}

void CLVersionCheck::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Version);

    __END_CATCH
}

string CLVersionCheck::toString() const

{
    StringStream msg;
    msg << "CLVersionCheck(" << "Version:" << (int)m_Version << ")";
    return msg.toString();
}
