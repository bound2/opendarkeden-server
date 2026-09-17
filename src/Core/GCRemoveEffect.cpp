//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRemoveEffect.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCRemoveEffect.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCRemoveEffect::GCRemoveEffect()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCRemoveEffect::~GCRemoveEffect()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCRemoveEffect::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);

    BYTE listNum = 0;
    iStream.read(listNum);
    if (listNum > kMaxCount)
        throw InvalidProtocolException("too many effects in the list");

    m_EffectList.clear();

    EffectID_t value;
    for (int i = 0; i < listNum; i++) {
        iStream.read(value);
        m_EffectList.push_back(value);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCRemoveEffect::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_EffectList.size() > kMaxCount)
        throw InvalidProtocolException("too many effects in the list");

    oStream.write(m_ObjectID);
    oStream.write((BYTE)m_EffectList.size());

    for (list<EffectID_t>::const_iterator itr = m_EffectList.begin(); itr != m_EffectList.end(); itr++) {
        oStream.write(*itr);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// GCRemoveEffect::addListElement()
//
// Member function that adds one (changed part, changed value) set to the list.
//
//////////////////////////////////////////////////////////////////////
void GCRemoveEffect::addEffectList(EffectID_t Value)

{
    __BEGIN_TRY

    if (m_EffectList.size() >= kMaxCount)
        throw InvalidProtocolException("too many effects in the list");

    m_EffectList.push_back(Value);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCRemoveEffect::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "GCRemoveEffect(" << ",ListNum:" << (int)m_EffectList.size() << ",ListSet(";
    for (list<EffectID_t>::const_iterator itr = m_EffectList.begin(); itr != m_EffectList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
