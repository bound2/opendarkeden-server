//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNPCAskVariable.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCNPCAskVariable.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
GCNPCAskVariable::GCNPCAskVariable()

    {__BEGIN_TRY

         __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
GCNPCAskVariable::~GCNPCAskVariable()

{
    __BEGIN_TRY

    clearScriptParameters();

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////////////
void GCNPCAskVariable::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_ScriptID);

    BYTE szParameters = 0;
    iStream.read(szParameters);

    clearScriptParameters();

    for (int i = 0; i < szParameters; i++) {
        ScriptParameter* pParam = new ScriptParameter();
        pParam->read(iStream);
        addScriptParameter(pParam);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////////////
void GCNPCAskVariable::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_ScriptParameters.size() > kMaxCount)
        throw InvalidProtocolException("too many script parameters");

    oStream.write(m_ObjectID);
    oStream.write(m_ScriptID);

    oStream.write((BYTE)m_ScriptParameters.size());

    HashMapScriptParameterConstItor itr = m_ScriptParameters.begin();
    for (; itr != m_ScriptParameters.end(); itr++) {
        (itr->second)->write(oStream);
    }

    __END_CATCH
}

void GCNPCAskVariable::addScriptParameter(ScriptParameter* pParam) {
    __BEGIN_TRY

    // The packet owns the record from here, so a refused one is destroyed.
    if (m_ScriptParameters.size() >= kMaxCount) {
        SAFE_DELETE(pParam);
        throw InvalidProtocolException("too many script parameters");
    }

    HashMapScriptParameterItor itr = m_ScriptParameters.find(pParam->getName());

    if (itr != m_ScriptParameters.end()) {
        SAFE_DELETE(pParam);
        throw DuplicatedException("Dup Parameter Variable name");
    }

    m_ScriptParameters[pParam->getName()] = pParam;

    __END_CATCH
}

void GCNPCAskVariable::clearScriptParameters()

{
    __BEGIN_TRY

    HashMapScriptParameterItor itr = m_ScriptParameters.begin();

    for (; itr != m_ScriptParameters.end(); itr++) {
        SAFE_DELETE(itr->second);
    }

    m_ScriptParameters.clear();

    __END_CATCH
}

string GCNPCAskVariable::getValue(const string& name) const {
    __BEGIN_TRY

    HashMapScriptParameterConstItor itr = m_ScriptParameters.find(name);

    if (itr == m_ScriptParameters.end()) {
        // name �� ���� ����. NoSuchElement �� �������ϳ�
        // �� name �� �����ֵ��� �Ѵ�.
        return name;
    }

    return (itr->second)->getValue();

    __END_CATCH
}


PacketSize_t GCNPCAskVariable::getPacketSize() const

{
    __BEGIN_TRY

    PacketSize_t result = 0;

    result += szObjectID + szScriptID + szBYTE;

    HashMapScriptParameterConstItor itr = m_ScriptParameters.begin();

    for (; itr != m_ScriptParameters.end(); itr++) {
        result += (itr->second)->getSize();
    }

    return result;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCNPCAskVariable::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCNPCAskVariable(" << "ObjectID:" << m_ObjectID << ",ScriptID: " << m_ScriptID << ",Parameters: (";

    HashMapScriptParameterConstItor itr = m_ScriptParameters.begin();
    for (; itr != m_ScriptParameters.end(); itr++) {
        msg << (itr->second)->toString() << ", ";
    }
    msg << ")";

    return msg.toString();

    __END_CATCH
}
