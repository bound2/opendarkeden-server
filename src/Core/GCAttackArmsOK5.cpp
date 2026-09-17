//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAttackArmsOK5.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCAttackArmsOK5.h"

#include "types/SkillTypes.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCAttackArmsOK5::GCAttackArmsOK5()

{
    __BEGIN_TRY

    m_SkillType = SKILL_ATTACK_ARMS;
    m_ObjectID = 0;
    m_TargetObjectID = 0;
    m_bSuccess = false;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCAttackArmsOK5::~GCAttackArmsOK5()

{
    __BEGIN_TRY
    __BEGIN_DEBUG
    __END_DEBUG
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCAttackArmsOK5::read(SocketInputStream& iStream)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // State the actual size when optimizing.
    iStream.read(m_SkillType);
    iStream.read(m_ObjectID);
    iStream.read(m_TargetObjectID);
    //	iStream.read(m_X);
    //	iStream.read(m_Y);

    // The hit flag is one byte on the wire; any non-zero value is a hit.
    BYTE success;
    iStream.read(success);
    m_bSuccess = (success != 0);

    __END_DEBUG
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCAttackArmsOK5::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY
    __BEGIN_DEBUG

    // State the actual size when optimizing.
    oStream.write(m_SkillType);
    oStream.write(m_ObjectID);
    oStream.write(m_TargetObjectID);
    //	oStream.write(m_X);
    //	oStream.write(m_Y);
    oStream.write(m_bSuccess);

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCAttackArmsOK5::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCAttackArmsOK5(" << "SkillType:" << (int)m_SkillType << "ObjectID:" << (int)m_ObjectID
        << ",TargetObjectID:" << (int)m_TargetObjectID << ",Success:" << (int)m_bSuccess << ")";
    return msg.toString();

    __END_CATCH
}
