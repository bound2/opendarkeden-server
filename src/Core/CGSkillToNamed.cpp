//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSkillToNamed.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGSkillToNamed.h"

#include "WireString.h"

CGSkillToNamed::CGSkillToNamed()

    {__BEGIN_TRY __END_CATCH}

CGSkillToNamed::~CGSkillToNamed()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void CGSkillToNamed::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read((char*)&m_SkillType, szSkillType);
    iStream.read((char*)&m_CEffectID, szCEffectID);
    de::wire::readString(iStream, m_TargetName, {1, 20}, "TargetName");

    __END_CATCH
}

void CGSkillToNamed::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write((char*)&m_SkillType, szSkillType);
    oStream.write((char*)&m_CEffectID, szCEffectID);
    de::wire::writeString(oStream, m_TargetName, {1, 20}, "TargetName");

    __END_CATCH
}


string CGSkillToNamed::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGSkillToNamed(" << "SkillType:" << (int)m_SkillType << ",CEffectID:" << (int)m_CEffectID
        << ",TargetName:" << m_TargetName << ")";
    return msg.toString();

    __END_CATCH
}
