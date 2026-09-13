//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSMSSend.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGSMSSend.h"

#include "SocketEncryptInputStream.h"
#include "SocketEncryptOutputStream.h"
#include "WireString.h"


void CGSMSSend::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    BYTE size;

    iStream.read(size);
    if (size > MAX_RECEVIER_NUM)
        throw InvalidProtocolException("too many receivers");

    m_Numbers.clear();
    for (int i = 0; i < size; ++i) {
        string number;
        de::wire::readString(iStream, number, {0, MAX_NUMBER_LENGTH}, "Number");
        m_Numbers.push_back(number);
    }

    de::wire::readString(iStream, m_CallerNumber, {0, MAX_NUMBER_LENGTH}, "CallerNumber");
    de::wire::readString(iStream, m_Message, {0, MAX_MESSAGE_LENGTH}, "Message");

    __END_CATCH
}

void CGSMSSend::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_Numbers.size() > MAX_RECEVIER_NUM)
        throw InvalidProtocolException("too many receivers");

    BYTE size = m_Numbers.size();
    oStream.write(size);

    list<string>::const_iterator itr = m_Numbers.begin();
    list<string>::const_iterator endItr = m_Numbers.end();

    for (; itr != endItr; ++itr) {
        de::wire::writeString(oStream, *itr, {0, MAX_NUMBER_LENGTH}, "Number");
    }

    de::wire::writeString(oStream, m_CallerNumber, {0, MAX_NUMBER_LENGTH}, "CallerNumber");
    de::wire::writeString(oStream, m_Message, {0, MAX_MESSAGE_LENGTH}, "Message");

    __END_CATCH
}

PacketSize_t CGSMSSend::getPacketSize() const {
    __BEGIN_TRY

    PacketSize_t ret = szBYTE;

    list<string>::const_iterator itr = m_Numbers.begin();
    list<string>::const_iterator endItr = m_Numbers.end();

    for (; itr != endItr; ++itr) {
        ret += de::wire::stringWireSize(*itr);
    }

    ret += de::wire::stringWireSize(m_CallerNumber);
    ret += de::wire::stringWireSize(m_Message);

    return ret;

    __END_CATCH
}

string CGSMSSend::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGSMSSend(" << "Caller : " << m_CallerNumber << ", Receivers : (";

    list<string>::const_iterator itr = m_Numbers.begin();
    list<string>::const_iterator endItr = m_Numbers.end();

    for (; itr != endItr; ++itr) {
        msg << *itr << ", ";
    }

    msg << "), Message : " << m_Message << ")";

    return msg.toString();

    __END_CATCH
}
