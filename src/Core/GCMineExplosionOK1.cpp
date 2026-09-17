//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMineExplosionOK1.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCMineExplosionOK1.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCMineExplosionOK1::GCMineExplosionOK1()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCMineExplosionOK1::~GCMineExplosionOK1()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCMineExplosionOK1::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
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

    ModifyInfo::read(iStream);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCMineExplosionOK1::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_CList.size() > kMaxCount)
        throw InvalidProtocolException("too many creatures in the list");

    // State the actual size when optimizing.
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Dir);
    oStream.write(m_ItemType);
    oStream.write((BYTE)m_CList.size());

    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        oStream.write(*itr);
    }

    ModifyInfo::write(oStream);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// GCMineExplosionOK1::addListElement()
//
// Member function that adds one (changed part, changed value) set to the list.
//
//////////////////////////////////////////////////////////////////////
void GCMineExplosionOK1::addCListElement(ObjectID_t ObjectID)

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
string GCMineExplosionOK1::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMineExplosionOK1(" << "X:" << (int)m_X << ",Y:" << (int)m_Y << ",Dir:" << (int)m_Dir
        << ",ItemType:" << (int)m_ItemType << ",CListNum: " << (int)m_CList.size() << " CListSet(";

    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }

    msg << ")";
    msg << ModifyInfo::toString();

    msg << ")";


    return msg.toString();

    __END_CATCH
}
