//////////////////////////////////////////////////////////////////////
//
// Filename    : GCKnocksTargetBackOK2.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

#include "GCKnocksTargetBackOK2.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCKnocksTargetBackOK2::GCKnocksTargetBackOK2()

    {__BEGIN_TRY


         __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCKnocksTargetBackOK2::~GCKnocksTargetBackOK2()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCKnocksTargetBackOK2::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_SkillType);
    iStream.read(m_dir);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_ObjectID);

    ModifyInfo::read(iStream);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCKnocksTargetBackOK2::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_SkillType);
    oStream.write(m_dir);
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_ObjectID);

    ModifyInfo::write(oStream);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCKnocksTargetBackOK2::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "GCKnocksTargetBackOK2(" << "ObjectID: " << (int)m_ObjectID << "(Dir,X,Y):" << (int)m_dir << "," << (int)m_X
        << "," << (int)m_Y << ModifyInfo::toString() << ")";
    return msg.toString();

    __END_CATCH
}
