//////////////////////////////////////////////////////////////////////////////
// Filename    : CGCrashReport.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGCrashReport.h"

#include "WireString.h"

CGCrashReport::CGCrashReport()

    {__BEGIN_TRY __END_CATCH}

CGCrashReport::~CGCrashReport()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void CGCrashReport::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ExecutableTime, kExecutableTimeLength);
    iStream.read(m_Version);
    iStream.read(m_Address, kAddressLength);

    // write() admits all three empty, so read() takes what it emits.
    de::wire::readString16(iStream, m_OS, {0, 100}, "OS");
    de::wire::readString16(iStream, m_CallStack, {0, 1024}, "CallStack");
    de::wire::readString16(iStream, m_Message, {0, 1024}, "Message");

    __END_CATCH
}

void CGCrashReport::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_ExecutableTime.size() != kExecutableTimeLength)
        throw InvalidProtocolException("ExecutableTime is not the fixed width");

    if (m_Address.size() != kAddressLength)
        throw InvalidProtocolException("Address is not the fixed width");

    oStream.write(m_ExecutableTime);
    oStream.write(m_Version);
    oStream.write(m_Address);

    de::wire::writeString16(oStream, m_OS, {0, 100}, "OS");
    de::wire::writeString16(oStream, m_CallStack, {0, 1024}, "CallStack");
    de::wire::writeString16(oStream, m_Message, {0, 1024}, "Message");

    __END_CATCH
}

string CGCrashReport::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGCrashReport(" << m_ExecutableTime << ", " << m_Version << ", " << m_Address << ", " << m_OS << ", "
        << m_CallStack << ", " << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
