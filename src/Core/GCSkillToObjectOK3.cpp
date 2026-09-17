//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToObjectOK3.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCSkillToObjectOK3.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSkillToObjectOK3::GCSkillToObjectOK3() {
    __BEGIN_TRY

    m_ObjectID = 0;
    m_SkillType = 0;
    m_TargetX = 0;
    m_TargetY = 0;
    m_Grade = 0;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSkillToObjectOK3::~GCSkillToObjectOK3()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSkillToObjectOK3::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_ObjectID);
    iStream.read(m_SkillType);
    iStream.read(m_TargetX);
    iStream.read(m_TargetY);
    iStream.read(m_Grade);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCSkillToObjectOK3::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_ObjectID);
    oStream.write(m_SkillType);
    oStream.write(m_TargetX);
    oStream.write(m_TargetY);
    oStream.write(m_Grade);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCSkillToObjectOK3::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCSkillToObjectOK3(" << "SkillType:" << (int)m_SkillType << ",ObjectID:" << (int)m_ObjectID
        << ",TargetXY:" << m_TargetX << "," << m_TargetY << ",Grade:" << m_Grade << ")";
    return msg.toString();

    __END_CATCH
}
