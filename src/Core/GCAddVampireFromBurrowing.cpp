//----------------------------------------------------------------------
//
// Filename    : GCAddVampireFromBurrowing.cpp
// Written By  : Reiot
//
//----------------------------------------------------------------------

// include files
#include "GCAddVampireFromBurrowing.h"

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------
GCAddVampireFromBurrowing::~GCAddVampireFromBurrowing() noexcept

{
    SAFE_DELETE(m_pEffectInfo);
}

//----------------------------------------------------------------------
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//----------------------------------------------------------------------
void GCAddVampireFromBurrowing::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    m_VampireInfo.read(iStream);

    m_pEffectInfo = new EffectInfo();
    m_pEffectInfo->read(iStream);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//--------------------------------------------------------------------------------
void GCAddVampireFromBurrowing::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    m_VampireInfo.write(oStream);

    // A packet carrying no effect record puts an empty list on the wire.
    EffectInfo noEffects;
    const EffectInfo& effects = (m_pEffectInfo != NULL) ? *m_pEffectInfo : noEffects;
    effects.write(oStream);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCAddVampireFromBurrowing::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCAddVampireFromBurrowing(" << "VampireInfo:" << m_VampireInfo.toString()
        << "EffectInfo:" << ((m_pEffectInfo != NULL) ? m_pEffectInfo->toString() : "NULL") << ")";

    return msg.toString();

    __END_CATCH
}
