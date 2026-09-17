//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAttackArmsOK1.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCAttackArmsOK1.h"

#include "types/SkillTypes.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCAttackArmsOK1::GCAttackArmsOK1()

{
    __BEGIN_TRY

    m_SkillType = SKILL_ATTACK_ARMS;
    m_ObjectID = 0;
    m_BulletNum = 0;
    m_bSuccess = false;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCAttackArmsOK1::~GCAttackArmsOK1()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    __END_DEBUG
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCAttackArmsOK1::read(SocketInputStream& iStream)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    iStream.read(m_SkillType);
    iStream.read(m_ObjectID);
    iStream.read(m_BulletNum);

    // The hit flag is one byte on the wire; any non-zero value is a hit.
    BYTE success;
    iStream.read(success);
    m_bSuccess = (success != 0);

    ModifyInfo::read(iStream);

    __END_DEBUG
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCAttackArmsOK1::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY
    __BEGIN_DEBUG

    oStream.write(m_SkillType);
    oStream.write(m_ObjectID);
    oStream.write(m_BulletNum);
    oStream.write(m_bSuccess);

    ModifyInfo::write(oStream);

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCAttackArmsOK1::toString() const {
    __BEGIN_TRY
    __BEGIN_DEBUG

    StringStream msg;
    msg << "GCAttackArmsOK1(" << "SkillType:" << (int)m_SkillType << "ObjectID:" << (int)m_ObjectID
        << ",BulletNum:" << (int)m_BulletNum << ",Success:" << (int)m_bSuccess << ModifyInfo::toString() << ")";
    return msg.toString();

    __END_DEBUG
    __END_CATCH
}
