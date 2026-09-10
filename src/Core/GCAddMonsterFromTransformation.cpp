//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAddMonsterFromTransformation.cpp
// Written By  : Reiot
//
//////////////////////////////////////////////////////////////////////

#include "GCAddMonsterFromTransformation.h"

#include "WireString.h"

//--------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------
GCAddMonsterFromTransformation::GCAddMonsterFromTransformation()

    {__BEGIN_TRY __END_CATCH}

//--------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------
GCAddMonsterFromTransformation::~GCAddMonsterFromTransformation() noexcept

{
    SAFE_DELETE(m_pEffectInfo);
}

//////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////
void GCAddMonsterFromTransformation::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_MonsterType);

    de::wire::readString(iStream, m_MonsterName, {0, kMaxNameSize}, "MonsterName");

    iStream.read(m_MainColor);
    iStream.read(m_SubColor);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Dir);

    // The record the packet already holds is replaced, not leaked.
    SAFE_DELETE(m_pEffectInfo);
    m_pEffectInfo = new EffectInfo();
    m_pEffectInfo->read(iStream);

    iStream.read(m_CurrentHP);
    iStream.read(m_MaxHP);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////
void GCAddMonsterFromTransformation::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_MonsterType);

    de::wire::writeString(oStream, m_MonsterName, {0, kMaxNameSize}, "MonsterName");

    oStream.write(m_MainColor);
    oStream.write(m_SubColor);
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Dir);

    // A packet carrying no effect record puts an empty list on the wire.
    EffectInfo noEffects;
    const EffectInfo& effects = (m_pEffectInfo != NULL) ? *m_pEffectInfo : noEffects;
    effects.write(oStream);

    oStream.write(m_CurrentHP);
    oStream.write(m_MaxHP);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCAddMonsterFromTransformation::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCAddMonsterFromTransformation(" << "ObjectID:" << (int)m_ObjectID << ",MonsterType:" << (int)m_MonsterType
        << ",MonsterName:" << m_MonsterName << ",MainColor:" << (int)m_MainColor << ",SubColor:" << (int)m_SubColor
        << ",X:" << (int)m_X << ",Y:" << (int)m_Y << ",Dir:" << dir2String(m_Dir)
        << ",Effects:" << ((m_pEffectInfo != NULL) ? m_pEffectInfo->toString() : "NULL")
        << ",CurrentHP:" << (int)m_CurrentHP << ",MaxHP:" << (int)m_MaxHP << ")";

    return msg.toString();

    __END_CATCH
}
