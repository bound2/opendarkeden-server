//////////////////////////////////////////////////////////////////////////////
// Filename    : CGTypeStringList.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGTypeStringList.h"

#include "WireString.h"

CGTypeStringList::CGTypeStringList()

    {__BEGIN_TRY __END_CATCH}

CGTypeStringList::~CGTypeStringList()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void CGTypeStringList::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    clearString();

    BYTE num;

    // A byte carries more values than there are list types, so it is
    // tested before it is stored.
    BYTE type = 0;
    iStream.read(type);

    if (type > STRING_TYPE_FORCE_APART_COUPLE)
        throw InvalidProtocolException("list type out of range");

    m_StringType = type;
    iStream.read(num);

    if (num > kMaxStringCount)
        throw InvalidProtocolException("too many list strings");

    for (BYTE i = 0; i < num; i++) {
        string temp;
        de::wire::readString(iStream, temp, {1, MAX_STRING_LENGTH}, "list string");
        addString(temp);
    }

    iStream.read(m_Param);

    __END_CATCH
}

void CGTypeStringList::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_StringType);

    if (m_StringList.size() > kMaxStringCount)
        throw InvalidProtocolException("too many list strings");

    BYTE szList = m_StringList.size();

    oStream.write(szList);

    list<string>::const_iterator itr = m_StringList.begin();

    for (; itr != m_StringList.end(); ++itr) {
        de::wire::writeString(oStream, *itr, {1, MAX_STRING_LENGTH}, "list string");
    }

    oStream.write(m_Param);

    __END_CATCH
}

string CGTypeStringList::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGTypeStringList(" << ")";
    return msg.toString();

    __END_CATCH
}
