//////////////////////////////////////////////////////////////////////////////
// Filename    : CLDeletePC.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLDeletePC.h"

#include "WireString.h"

void CLDeletePC::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_Name, {1, 20}, "Name");

    BYTE slot;
    iStream.read(slot);

    // SLOT_MAX is the count of slots, not a slot, and Slot2String has one
    // entry per real slot. The byte is checked before it becomes a Slot:
    // an enum object holding a value outside its enumeration is undefined
    // to load, so a guard placed after the assignment could not be reached
    // with the input it exists to refuse.
    if (slot >= (BYTE)SLOT_MAX)
        throw InvalidProtocolException("slot out of range");

    m_Slot = Slot(slot);

    de::wire::readString(iStream, m_SSN, {1, 14}, "SSN");

    __END_CATCH
}

void CLDeletePC::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");

    oStream.write((BYTE)m_Slot);

    de::wire::writeString(oStream, m_SSN, {1, 14}, "SSN");

    __END_CATCH
}

string CLDeletePC::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CLDeletePC(Name:" << m_Name << ",Slot:" << (int)m_Slot << ",SSN:" << m_SSN << ")";
    return msg.toString();

    __END_CATCH
}
