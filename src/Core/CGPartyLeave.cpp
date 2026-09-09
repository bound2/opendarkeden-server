//////////////////////////////////////////////////////////////////////////////
// Filename    : CGPartyLeave.cpp
// Written By  : 김성민
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGPartyLeave.h"

void CGPartyLeave::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    BYTE name_length = 0;
    iStream.read(name_length);

    // An empty name asks to leave the party rather than to expel a member.
    if (name_length > 10)
        throw InvalidProtocolException("too long target name length");

    if (name_length > 0) {
        iStream.read(m_TargetName, name_length);
    }

    __END_CATCH
}

void CGPartyLeave::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_TargetName.size() > 10)
        throw InvalidProtocolException("too long target name length");

    BYTE name_length = m_TargetName.size();
    oStream.write(name_length);
    if (name_length > 0) {
        oStream.write(m_TargetName);
    }

    __END_CATCH
}

string CGPartyLeave::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGPartyLeave(" << "TargetName:" << m_TargetName << ")";
    return msg.toString();

    __END_CATCH
}
