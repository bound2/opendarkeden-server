//////////////////////////////////////////////////////////////////////
//
// Filename    : GCKnocksTargetBackOK4.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCKnocksTargetBackOK4.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCKnocksTargetBackOK4::GCKnocksTargetBackOK4()

    {__BEGIN_TRY

         __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCKnocksTargetBackOK4::~GCKnocksTargetBackOK4()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCKnocksTargetBackOK4::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // State the actual size when optimizing.
    //	iStream.read(m_ObjectID);
    iStream.read(m_TargetObjectID);
    //	iStream.read(m_X);
    //	iStream.read(m_Y);
    //	iStream.read(m_bSuccess);

    iStream.read(m_SkillType);
    iStream.read(m_dir);
    iStream.read(m_X);
    iStream.read(m_Y);
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCKnocksTargetBackOK4::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // State the actual size when optimizing.
    //	oStream.write(m_ObjectID);
    oStream.write(m_TargetObjectID);
    //	oStream.write(m_X);
    //	oStream.write(m_Y);
    //	oStream.write(m_bSuccess);

    oStream.write(m_SkillType);
    oStream.write(m_dir);
    oStream.write(m_X);
    oStream.write(m_Y);
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCKnocksTargetBackOK4::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCKnocksTargetBackOK4(" << "TargetObjectID:" << (int)m_TargetObjectID << "(Dir,X,Y): " << (int)m_dir << ","
        << (int)m_X << "," << (int)m_Y << ")";
    return msg.toString();

    __END_CATCH
}
