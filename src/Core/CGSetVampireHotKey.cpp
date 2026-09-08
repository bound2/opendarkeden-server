//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSetVampireHotKey.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGSetVampireHotKey.h"

void CGSetVampireHotKey::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    for (int i = 0; i < 8; i++) {
        iStream.read(m_HotKey[i]);
    }

    __END_CATCH
}

void CGSetVampireHotKey::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    for (int i = 0; i < 8; i++) {
        oStream.write(m_HotKey[i]);
    }

    __END_CATCH
}

string CGSetVampireHotKey::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGSetVampireHotKey(";
    for (int i = 0; i < 8; i++) {
        if (i != 0)
            msg << ",";
        msg << "F" << i + 5 << ":" << (int)m_HotKey[i];
    }
    msg << ")";

    return msg.toString();

    __END_CATCH
}
