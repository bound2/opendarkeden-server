//////////////////////////////////////////////////////////////////////
//
// Filename    : GCThrowBombOK3.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCThrowBombOK3.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCThrowBombOK3::GCThrowBombOK3()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCThrowBombOK3::~GCThrowBombOK3()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCThrowBombOK3::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_ObjectID);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Dir);
    iStream.read(m_ItemType);

    BYTE CListNum = 0;
    iStream.read(CListNum);

    m_CList.clear();

    ObjectID_t m_Value;
    int i;

    for (i = 0; i < CListNum; i++) {
        iStream.read(m_Value);
        m_CList.push_back(m_Value);
    }


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCThrowBombOK3::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_CList.size() > kMaxCount)
        throw InvalidProtocolException("too many creatures in the list");

    // State the actual size when optimizing.
    oStream.write(m_ObjectID);
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Dir);
    oStream.write(m_ItemType);
    oStream.write((BYTE)m_CList.size());

    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        oStream.write(*itr);
    }


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// GCThrowBombOK3::addListElement()
//
// Member function that adds one (changed part, changed value) set to the list.
//
//////////////////////////////////////////////////////////////////////
void GCThrowBombOK3::addCListElement(ObjectID_t ObjectID)

{
    __BEGIN_TRY

    if (m_CList.size() >= kMaxCount)
        throw InvalidProtocolException("too many creatures in the list");

    m_CList.push_back(ObjectID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCThrowBombOK3::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCThrowBombOK3(ObjectID:" << (int)m_ObjectID << ",X:" << (int)m_X << ",Y:" << (int)m_Y
        << ",Dir:" << (int)m_Dir << ",ItemType:" << (int)m_ItemType << ",CListNum: " << (int)m_CList.size()
        << " CListSet(";

    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
