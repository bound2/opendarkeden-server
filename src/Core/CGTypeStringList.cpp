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

    iStream.read(m_StringType);
    //	cout << "CGTypeStringList(StringType:" << (int)m_StringType << ", ";
    iStream.read(num);
    //	cout << "Number of String:" << (int)num << ", ";

    for (BYTE i = 0; i < num; i++) {
        string temp;
        de::wire::readString(iStream, temp, {1, MAX_STRING_LENGTH}, "list string");
        addString(temp);
    }

    iStream.read(m_Param);
    //	cout << "Parameter : " << m_Param << " )" << endl;

    __END_CATCH
}

void CGTypeStringList::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_StringType);

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
