//////////////////////////////////////////////////////////////////////
//
// Filename    : LCRegisterPlayerOK.cpp
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "LCRegisterPlayerOK.h"

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void LCRegisterPlayerOK::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    BYTE szGroupName;
    iStream.read(szGroupName);

    if (szGroupName == 0)
        throw InvalidProtocolException("szGroupName == 0");

    if (szGroupName > maxNameLength)
        throw InvalidProtocolException("too long group name length");

    iStream.read(m_GroupName, szGroupName);
    iStream.read(m_isAdult);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void LCRegisterPlayerOK::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    BYTE szGroupName = m_GroupName.size();

    if (szGroupName == 0)
        throw InvalidProtocolException("szGroupName == 0");

    if (szGroupName > maxNameLength)
        throw InvalidProtocolException("too long group name length");

    oStream.write(szGroupName);
    oStream.write(m_GroupName);
    oStream.write(m_isAdult);

    __END_CATCH
}
