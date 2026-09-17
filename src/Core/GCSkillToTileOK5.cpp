//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToTileOK5.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCSkillToTileOK5.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSkillToTileOK5::GCSkillToTileOK5() {
    __BEGIN_TRY

    m_ObjectID = 0;
    m_SkillType = 0;
    m_X = 0;
    m_Y = 0;
    m_Range = 0;
    m_Duration = 0;
    m_Grade = 0;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSkillToTileOK5::~GCSkillToTileOK5()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK5::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_ObjectID);
    iStream.read(m_SkillType);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Range);
    iStream.read(m_Duration);
    iStream.read(m_Grade);
    BYTE CListNum;
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
void GCSkillToTileOK5::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_CList.size() > kMaxCount)
        throw InvalidProtocolException("too many creatures in the list");

    // State the actual size when optimizing.
    oStream.write(m_ObjectID);
    oStream.write(m_SkillType);
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Range);
    oStream.write(m_Duration);
    oStream.write(m_Grade);
    oStream.write((BYTE)m_CList.size());

    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        oStream.write(*itr);
    }


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// GCSkillToTileOK5::addListElement()
//
// Member function that adds one (changed part, changed value) set to the list.
//
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK5::addCListElement(ObjectID_t ObjectID)

{
    __BEGIN_TRY

    if (m_CList.size() >= kMaxCount)
        throw InvalidProtocolException("too many creatures in the list");

    m_CList.push_back(ObjectID);

    __END_CATCH
}

/*
//////////////////////////////////////////////////////////////////////
//
// GCSkillToTileOK5::deleteCListElement()
//
// Member function needed when removing an element of the creature list.
//
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK5::deleteCListElement()

{
    __BEGIN_TRY

    // Drop one creature id.
    m_CList.pop_front();

    // Drop one from the creature list counter.
    m_CListNum--;

    __END_CATCH
}
*/


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCSkillToTileOK5::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCSkillToTileOK5(" << "ObjectID:" << (int)m_ObjectID << ",SkillType:" << (int)m_SkillType
        << ",X:" << (int)m_X << ",Y:" << (int)m_Y << ",Range:" << (int)m_Range << ",Duration: " << (int)m_Duration
        << ",Grade: " << (int)m_Grade << ",CListNum: " << (int)m_CList.size() << " CListSet(";

    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
