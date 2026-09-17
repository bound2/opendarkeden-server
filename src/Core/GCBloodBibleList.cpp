//////////////////////////////////////////////////////////////////////////////
// Filename    : GCBloodBibleList.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCBloodBibleList.h"

#include "Assert1.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
GCBloodBibleList::~GCBloodBibleList()

{
    __BEGIN_TRY

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////////////
void GCBloodBibleList::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The listing replaces the one the packet holds.
    m_BloodBibleList.clear();

    BYTE num;

    iStream.read(num);

    if (num > kMaxEntries)
        throw InvalidProtocolException("too many blood bibles");

    for (int i = 0; i < num; ++i) {
        ItemType_t iType;
        iStream.read(iType);

        m_BloodBibleList.push_back(iType);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////////////
void GCBloodBibleList::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_BloodBibleList.size() > kMaxEntries)
        throw InvalidProtocolException("too many blood bibles");

    BYTE num = m_BloodBibleList.size();

    oStream.write(num);

    vector<ItemType_t>::const_iterator itr = m_BloodBibleList.begin();

    for (int i = 0; i < num; i++) {
        oStream.write(*itr++);
    }

    __END_CATCH
}


PacketSize_t GCBloodBibleList::getPacketSize() const

{
    __BEGIN_TRY

    PacketSize_t result = 0;

    result += szBYTE + szItemType * m_BloodBibleList.size();

    return result;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCBloodBibleList::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCBloodBibleList(";
    msg << ")";

    return msg.toString();

    __END_CATCH
}
