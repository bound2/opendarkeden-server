//////////////////////////////////////////////////////////////////////////////
// Filename    : CLSelectPC.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLSelectPC.h"

void CLSelectPC::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // read creature's name
    BYTE szPCName;

    iStream.read(szPCName);

    if (szPCName == 0)
        throw InvalidProtocolException("szPCName == 0");

    if (szPCName > 20)
        throw InvalidProtocolException("too long pc name length");

    iStream.read(m_PCName, szPCName);

    // read pc type
    BYTE pcType;
    iStream.read(pcType);

    // PC_OUSTERS is the last PCType (the enum has no count enumerator) and
    // PCType2String has one entry per type. The byte is checked before it
    // becomes a PCType: an enum object holding a value outside its
    // enumeration is undefined to load, so a comparison placed after the
    // assignment could not be reached with the input it exists to refuse.
    if (pcType > (BYTE)PC_OUSTERS)
        throw InvalidProtocolException("pc type out of range");

    m_PCType = PCType(pcType);

    __END_CATCH
}

void CLSelectPC::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // write creature's name
    BYTE szPCName = m_PCName.size();

    if (szPCName == 0)
        throw InvalidProtocolException("szPCName == 0");

    if (szPCName > 20)
        throw InvalidProtocolException("too long pc name length");

    oStream.write(szPCName);

    oStream.write(m_PCName);

    // write pc type
    if (m_PCType != PC_SLAYER && m_PCType != PC_VAMPIRE && m_PCType != PC_OUSTERS)
        throw InvalidProtocolException("invalid pc type");

    oStream.write((BYTE)m_PCType);

    __END_CATCH
}

// get packet's debug string
string CLSelectPC::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CLSelectPC(" << "PCName:" << m_PCName << ",PCType:" << PCType2String[m_PCType] << ")";
    return msg.toString();

    __END_CATCH
}
