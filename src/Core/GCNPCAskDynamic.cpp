//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNPCAskDynamic.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCNPCAskDynamic.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
GCNPCAskDynamic::GCNPCAskDynamic()

    {__BEGIN_TRY __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
GCNPCAskDynamic::~GCNPCAskDynamic()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////////////
void GCNPCAskDynamic::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    WORD size = 0;

    iStream.read(m_ObjectID);
    iStream.read(m_ScriptID);

    iStream.read(size);
    if (size == 0)
        throw InvalidProtocolException("subject size == 0");
    iStream.read(m_Subject, size);

    // read contents count
    BYTE contentsCount = 0;
    iStream.read(contentsCount);
    if (contentsCount > kMaxCount)
        throw InvalidProtocolException("too many contents");

    m_Contents.clear();

    for (int i = 0; i < contentsCount; i++) {
        // Read the string length, then the string itself. A zero length is
        // an empty choice, which write() emits as its bare length word.
        iStream.read(size);

        string msg = "";
        if (size > 0)
            iStream.read(msg, size);
        m_Contents.push_back(msg);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////////////
void GCNPCAskDynamic::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    WORD size = 0;

    if (m_Contents.size() > kMaxCount)
        throw InvalidProtocolException("too many contents");

    oStream.write(m_ObjectID);
    oStream.write(m_ScriptID);

    size = m_Subject.size();
    if (size == 0)
        throw InvalidProtocolException("subject size == 0");
    oStream.write(size);
    oStream.write(m_Subject);

    oStream.write((BYTE)m_Contents.size());

    list<string>::const_iterator itr = m_Contents.begin();

    for (; itr != m_Contents.end(); itr++) {
        // The string length, then the string itself. An empty choice is its
        // length word alone.
        size = (*itr).size();
        oStream.write(size);

        if (size > 0)
            oStream.write(*itr);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCNPCAskDynamic::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    int i = 0;

    msg << "GCNPCAskDynamic(" << "ObjectID:" << m_ObjectID << ",ScriptID: " << m_ScriptID << ",Subject:" << m_Subject;

    list<string>::const_iterator itr = m_Contents.begin();
    for (; itr != m_Contents.end(); itr++) {
        msg << ",Contents[" << i++ << "]:" << *itr;
    }
    msg << ")";

    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void GCNPCAskDynamic::addContent(string content)

{
    __BEGIN_TRY

    if (m_Contents.size() >= kMaxCount)
        throw InvalidProtocolException("too many contents");

    m_Contents.push_back(content);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string GCNPCAskDynamic::popContent(void)

{
    __BEGIN_TRY

    string rValue = m_Contents.front();
    m_Contents.pop_front();
    return rValue;

    __END_CATCH
}
