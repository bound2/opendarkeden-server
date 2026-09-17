//////////////////////////////////////////////////////////////////////
//
// Filename    : GCKnocksTargetBackOK1.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCKnocksTargetBackOK1.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCKnocksTargetBackOK1::GCKnocksTargetBackOK1()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCKnocksTargetBackOK1::~GCKnocksTargetBackOK1()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCKnocksTargetBackOK1::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_SkillType);
    iStream.read(m_dir);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_ObjectID);
    iStream.read(m_BulletNum);

    // A bool holds 0 or 1, so any other byte is refused rather than
    // stored in one.
    BYTE success = 0;
    iStream.read(success);

    if (success > 1)
        throw InvalidProtocolException("skill success flag is not a bool");

    m_bSuccess = (success != 0);

    ModifyInfo::read(iStream);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCKnocksTargetBackOK1::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write(m_SkillType);
    oStream.write(m_dir);
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_ObjectID);
    oStream.write(m_BulletNum);
    oStream.write(m_bSuccess);

    ModifyInfo::write(oStream);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCKnocksTargetBackOK1::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCKnocksTargetBackOK1(" << "ObjectID:" << (int)m_ObjectID << ",BulletNum:" << (int)m_BulletNum
        << ",(DIR,X,Y):" << (int)m_dir << (int)m_X << (int)m_Y << ",Success:" << (int)m_bSuccess
        << ModifyInfo::toString() << ")";
    return msg.toString();

    __END_CATCH
}
