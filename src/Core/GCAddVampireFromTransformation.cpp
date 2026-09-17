//----------------------------------------------------------------------
//
// Filename    : GCAddVampireFromTransformation.cpp
// Written By  : Reiot
//
//----------------------------------------------------------------------

// include files
#include "GCAddVampireFromTransformation.h"

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------
GCAddVampireFromTransformation::~GCAddVampireFromTransformation() noexcept

{
    SAFE_DELETE(m_pEffectInfo);
}

//----------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//----------------------------------------------------------------------
void GCAddVampireFromTransformation::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    m_VampireInfo.read(iStream);

    // The record the packet already holds is replaced, not leaked.
    SAFE_DELETE(m_pEffectInfo);
    m_pEffectInfo = new EffectInfo();
    m_pEffectInfo->read(iStream);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCAddVampireFromTransformation::write(SocketOutputStream& oStream) const

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
string GCAddVampireFromTransformation::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCAddVampireFromTransformation(" << "VampireInfo:" << m_VampireInfo.toString()
        << "EffectInfo:" << ((m_pEffectInfo != NULL) ? m_pEffectInfo->toString() : "NULL") << ")";

    return msg.toString();

    __END_CATCH
}
