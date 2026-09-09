//////////////////////////////////////////////////////////////////////////////
// Filename    : GCRemoveInjuriousCreature.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCRemoveInjuriousCreature.h"

#include "WireString.h"

void GCRemoveInjuriousCreature::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // 이름 읽기

    de::wire::readString(iStream, m_Name, {1, 10}, "Name");

    __END_CATCH
}

void GCRemoveInjuriousCreature::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // 이름 쓰기
    de::wire::writeString(oStream, m_Name, {1, 10}, "Name");

    __END_CATCH
}

string GCRemoveInjuriousCreature::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCRemoveInjuriousCreature(Name :" << m_Name << ")";
    return msg.toString();

    __END_CATCH
}
