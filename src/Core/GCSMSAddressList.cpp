//////////////////////////////////////////////////////////////////////
//
// Filename	: GCSMSAddressList.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//			   success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCSMSAddressList.h"

#include "WireString.h"

void AddressUnit::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(ElementID);
    de::wire::readString(iStream, CharacterName, {0, kMaxCharacterNameLength}, "CharacterName");
    de::wire::readString(iStream, CustomName, {0, kMaxCustomNameLength}, "CustomName");
    de::wire::readString(iStream, Number, {0, kMaxNumberLength}, "Number");

    __END_CATCH
}

void AddressUnit::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(ElementID);
    de::wire::writeString(oStream, CharacterName, {0, kMaxCharacterNameLength}, "CharacterName");
    de::wire::writeString(oStream, CustomName, {0, kMaxCustomNameLength}, "CustomName");
    de::wire::writeString(oStream, Number, {0, kMaxNumberLength}, "Number");

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSMSAddressList::GCSMSAddressList()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSMSAddressList::~GCSMSAddressList()

{
    __BEGIN_TRY

    clearAddresses();

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////
// Drop the listing, freeing every record it held.
//////////////////////////////////////////////////////////////////////
void GCSMSAddressList::clearAddresses()

{
    vector<AddressUnit*>::iterator itr = m_Addresses.begin();
    for (; itr != m_Addresses.end(); ++itr)
        delete *itr;

    m_Addresses.clear();
}

//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSMSAddressList::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The listing replaces the one the packet holds.
    clearAddresses();

    BYTE Num;
    iStream.read(Num);

    if (Num > MAX_ADDRESS_NUM)
        throw InvalidProtocolException("too many address book entries");

    for (int i = 0; i < Num; ++i) {
        AddressUnit* pUnit = new AddressUnit;
        m_Addresses.push_back(pUnit);
        pUnit->read(iStream);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCSMSAddressList::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_Addresses.size() > MAX_ADDRESS_NUM)
        throw InvalidProtocolException("too many address book entries");

    BYTE Num = m_Addresses.size();
    oStream.write(Num);

    vector<AddressUnit*>::const_iterator itr = m_Addresses.begin();
    vector<AddressUnit*>::const_iterator endItr = m_Addresses.end();

    for (; itr != endItr; ++itr) {
        (*itr)->write(oStream);
    }

    __END_CATCH
}

PacketSize_t GCSMSAddressList::getPacketSize() const {
    __BEGIN_TRY

    PacketSize_t ret = szBYTE;

    vector<AddressUnit*>::const_iterator itr = m_Addresses.begin();
    vector<AddressUnit*>::const_iterator endItr = m_Addresses.end();

    for (; itr != endItr; ++itr) {
        ret += (*itr)->getPacketSize();
    }

    return ret;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCSMSAddressList::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCSMSAddressList(" << ")";
    return msg.toString();

    __END_CATCH
}
