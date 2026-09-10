//////////////////////////////////////////////////////////////////////////////
// Filename    : CGModifyNickname.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////
#include "CGModifyNickname.h"

#include "WireString.h"

CGModifyNickname::CGModifyNickname()

    {__BEGIN_TRY __END_CATCH}

CGModifyNickname::~CGModifyNickname()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void CGModifyNickname::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ItemObjectID);

    de::wire::readString(iStream, m_Nickname, {0, MAX_NICKNAME_SIZE}, "Nickname");

    __END_CATCH
}

void CGModifyNickname::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ItemObjectID);

    de::wire::writeString(oStream, m_Nickname, {0, MAX_NICKNAME_SIZE}, "Nickname");

    __END_CATCH
}

string CGModifyNickname::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGModifyNickname(" << ")";
    return msg.toString();

    __END_CATCH
}
