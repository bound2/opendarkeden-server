//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillFailed2.cc
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet sent when a skill fails
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCSkillFailed2.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSkillFailed2::GCSkillFailed2() {
    __BEGIN_TRY

    m_ObjectID = 0;
    m_TargetObjectID = 0;
    m_SkillType = 0;
    m_Grade = 0;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSkillFailed2::~GCSkillFailed2()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSkillFailed2::read(SocketInputStream& iStream)

{
    __BEGIN_TRY
    iStream.read(m_ObjectID);
    iStream.read(m_TargetObjectID);
    iStream.read(m_SkillType);
    iStream.read(m_Grade);
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCSkillFailed2::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY
    oStream.write(m_ObjectID);
    oStream.write(m_TargetObjectID);
    oStream.write(m_SkillType);
    oStream.write(m_Grade);
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCSkillFailed2::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCSkillFailed2(" << "ObjectID:" << (int)m_ObjectID << ",TargetObjectID:" << (int)m_TargetObjectID
        << ",SkillType:" << (int)m_SkillType << ")";
    return msg.toString();

    __END_CATCH
}
