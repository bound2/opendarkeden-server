//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToInventoryOK1.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCSkillToInventoryOK1.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSkillToInventoryOK1::GCSkillToInventoryOK1() {
    __BEGIN_TRY

    m_SkillType = 0;
    m_ObjectID = 0;
    m_ItemType = 0;
    m_CEffectID = 0;
    m_Duration = 0;
    m_X = 0;
    m_Y = 0;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSkillToInventoryOK1::~GCSkillToInventoryOK1()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSkillToInventoryOK1::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_SkillType);
    iStream.read(m_ObjectID);
    iStream.read(m_ItemType);
    iStream.read(m_CEffectID);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Duration);

    ModifyInfo::read(iStream);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCSkillToInventoryOK1::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_SkillType);
    oStream.write(m_ObjectID);
    oStream.write(m_ItemType);
    oStream.write(m_CEffectID);
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Duration);
    ModifyInfo::write(oStream);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCSkillToInventoryOK1::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCSkillToInventoryOK1(" << "SkillType:" << (int)m_SkillType << ",ObjectID:" << (int)m_ObjectID
        << ",ItemType:" << (int)m_ItemType << ",CEffectID:" << (int)m_CEffectID << ",X:" << (int)m_X
        << ",Y:" << (int)m_Y << ",Duration: " << (int)m_Duration;
    msg << ModifyInfo::toString();
    msg << ")";

    return msg.toString();

    __END_CATCH
}
