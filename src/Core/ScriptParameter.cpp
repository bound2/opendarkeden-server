//////////////////////////////////////////////////////////////////////
//
// Filename    : ScriptParameter.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "ScriptParameter.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
ScriptParameter::ScriptParameter(){__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
ScriptParameter::~ScriptParameter() noexcept = default;


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void ScriptParameter::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    de::wire::readString(iStream, m_Name, {1, kMaxStringSize}, "Name");
    de::wire::readString(iStream, m_Value, {1, kMaxStringSize}, "Value");

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void ScriptParameter::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, kMaxStringSize}, "Name");
    de::wire::writeString(oStream, m_Value, {1, kMaxStringSize}, "Value");

    __END_CATCH
}

//--------------------------------------------------------------------
// getSize
//--------------------------------------------------------------------
PacketSize_t ScriptParameter::getSize() {
    __BEGIN_TRY

    return (PacketSize_t)(de::wire::stringWireSize(m_Name) + de::wire::stringWireSize(m_Value));

    __END_CATCH
}

/////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string ScriptParameter::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "ScriptParameter( " << "Name:" << m_Name << ",Value:" << m_Value << ")";

    return msg.toString();

    __END_CATCH
}
