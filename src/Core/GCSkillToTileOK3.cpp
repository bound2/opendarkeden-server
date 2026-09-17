//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToTileOK3.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCSkillToTileOK3.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSkillToTileOK3::GCSkillToTileOK3() {
    __BEGIN_TRY

    m_ObjectID = 0;
    m_SkillType = 0;
    m_X = 0;
    m_Y = 0;
    m_Grade = 0;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSkillToTileOK3::~GCSkillToTileOK3()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK3::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_ObjectID);
    iStream.read(m_SkillType);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Grade);
    /*
        iStream.read(m_Duration);
        iStream.read(m_CListNum);

        ObjectID_t m_Value;

        for(int i = 0; i < m_CListNum; i++ ) {
            iStream.read(m_Value);
            m_CList.push_back(m_Value);
        }
    */
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK3::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_ObjectID);
    oStream.write(m_SkillType);
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Grade);
    /*
        oStream.write(m_Duration);
        oStream.write(m_CListNum);

        for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr!= m_CList.end() ; itr++ ) {
            oStream.write(*itr);
        }
    */
    __END_CATCH
}
/*
//////////////////////////////////////////////////////////////////////
//
// GCSkillToTileOK3::addListElement()
//
// Member function that adds one (changed part, changed value) set to the list.
//
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK3::addCListElement(ObjectID_t ObjectID )

{
    __BEGIN_TRY

    // Add a creature ID.
    m_CList.push_back(ObjectID);

    // Raise the creature ID count.
    m_CListNum++;

    __END_CATCH

}
*/
/*
//////////////////////////////////////////////////////////////////////
//
// GCSkillToTileOK3::deleteCListElement()
//
// Member function needed when removing an element of the creature list.
//
//////////////////////////////////////////////////////////////////////
void GCSkillToTileOK3::deleteCListElement()

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
// GCSkillToTileOK3::execute()
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCSkillToTileOK3::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCSkillToTileOK3(" << "SkillType:" << (int)m_SkillType << ",ObjectID:" << (int)m_ObjectID
        << ",Grade:" << (int)m_Grade;
    msg << ")";
    return msg.toString();

    __END_CATCH
}
