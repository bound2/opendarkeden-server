//////////////////////////////////////////////////////////////////////
//
// Filename    : GCFastMove.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCFastMove.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCFastMove::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_FromX);
    iStream.read(m_FromY);
    iStream.read(m_ToX);
    iStream.read(m_ToY);
    iStream.read(m_SkillType);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCFastMove::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_FromX);
    oStream.write(m_FromY);
    oStream.write(m_ToX);
    oStream.write(m_ToY);
    oStream.write(m_SkillType);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCFastMove::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCFastMove(ObjectID:" << m_ObjectID << ",From(X,Y):" << (int)m_FromX << "," << (int)m_FromY
        << ",To(X,Y):" << (int)m_ToX << "," << (int)m_ToY << ",SkillType:" << m_SkillType << ")";
    return msg.toString();

    __END_CATCH
}
