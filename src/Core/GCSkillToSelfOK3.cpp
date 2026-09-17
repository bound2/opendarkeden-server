//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToSelfOK3.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCSkillToSelfOK3.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSkillToSelfOK3::GCSkillToSelfOK3() {
    __BEGIN_TRY

    m_X = 0;
    m_Y = 0;
    m_SkillType = 0;
    m_Duration = 0;
    m_Grade = 0;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSkillToSelfOK3::~GCSkillToSelfOK3()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSkillToSelfOK3::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_SkillType);
    iStream.read(m_Duration);
    iStream.read(m_Grade);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCSkillToSelfOK3::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_SkillType);
    oStream.write(m_Duration);
    oStream.write(m_Grade);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCSkillToSelfOK3::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCSkillToSelfOK3(" << "X,Y:" << (int)m_X << "," << (int)m_Y << ",SkillType:" << (int)m_SkillType
        << ",Duration:" << (int)m_Duration << ",Grade:" << (int)m_Grade << ")";
    return msg.toString();

    __END_CATCH
}
