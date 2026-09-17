//////////////////////////////////////////////////////////////////////
//
// Filename    : GCKnocksTargetBackOK5.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCKnocksTargetBackOK5.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCKnocksTargetBackOK5::GCKnocksTargetBackOK5()

    {__BEGIN_TRY __BEGIN_DEBUG

         __END_DEBUG __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCKnocksTargetBackOK5::~GCKnocksTargetBackOK5()

{
    __BEGIN_TRY
    __BEGIN_DEBUG
    __END_DEBUG
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCKnocksTargetBackOK5::read(SocketInputStream& iStream)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // State the actual size when optimizing.
    iStream.read(m_ObjectID);
    iStream.read(m_TargetObjectID);

    // A bool holds 0 or 1, so any other byte is refused rather than
    // stored in one.
    BYTE success = 0;
    iStream.read(success);

    if (success > 1)
        throw InvalidProtocolException("skill success flag is not a bool");

    m_bSuccess = (success != 0);

    iStream.read(m_SkillType);
    iStream.read(m_dir);
    iStream.read(m_X);
    iStream.read(m_Y);
    __END_DEBUG
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCKnocksTargetBackOK5::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY
    __BEGIN_DEBUG

    // State the actual size when optimizing.
    oStream.write(m_ObjectID);
    oStream.write(m_TargetObjectID);
    oStream.write(m_bSuccess);

    oStream.write(m_SkillType);
    oStream.write(m_dir);
    oStream.write(m_X);
    oStream.write(m_Y);

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCKnocksTargetBackOK5::toString() const {
    __BEGIN_TRY
    __BEGIN_DEBUG

    StringStream msg;
    msg << "GCKnocksTargetBackOK5(" << "ObjectID:" << (int)m_ObjectID << ",TargetObjectID: " << (int)m_TargetObjectID
        << ",Success:" << (int)m_bSuccess << ",(Dir,X,Y) : " << (int)m_dir << "," << (int)m_X << "," << (int)m_Y << ")";
    return msg.toString();

    __END_DEBUG
    __END_CATCH
}
