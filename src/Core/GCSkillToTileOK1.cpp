//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToTileOK1.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCSkillToTileOK1.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSkillToTileOK1::GCSkillToTileOK1() {
    __BEGIN_TRY

    m_SkillType = 0;
    m_CEffectID = 0;
    m_Duration = 0;
    m_Range = 0;
    m_X = 0;
    m_Y = 0;
    m_Grade = 0;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSkillToTileOK1::~GCSkillToTileOK1() {
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK1::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_SkillType);
    iStream.read(m_CEffectID);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Duration);
    iStream.read(m_Range);
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

    ModifyInfo::read(iStream);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK1::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_CList.size() > kMaxCount)
        throw InvalidProtocolException("too many creatures in the list");

    // State the actual size when optimizing.
    oStream.write(m_SkillType);
    oStream.write(m_CEffectID);
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Duration);
    oStream.write(m_Range);
    oStream.write(m_Grade);

    oStream.write((BYTE)m_CList.size());
    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        oStream.write(*itr);
    }

    ModifyInfo::write(oStream);
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// GCSkillToTileOK1::addListElement()
//
// Member function that adds one (changed part, changed value) set to the list.
//
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK1::addCListElement(ObjectID_t ObjectID)

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
// GCSkillToTileOK1::deleteCListElement()
//
// Member function needed when removing an element of the creature list.
//
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK1::deleteCListElement()

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
string GCSkillToTileOK1::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "GCSkillToTileOK1(" << "SkillType:" << (int)m_SkillType << ",CEffectID:" << (int)m_CEffectID
        << ",X:" << (int)m_X << ",Y:" << (int)m_Y << ",Duration:" << (int)m_Duration << ",Range:" << (int)m_Range
        << ",Grade:" << (int)m_Grade << ",CListNum:" << (int)m_CList.size() << "CListSet(";

    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }

    msg << ")";

    msg << ModifyInfo::toString();

    msg << ")";

    return msg.toString();

    __END_CATCH
}
