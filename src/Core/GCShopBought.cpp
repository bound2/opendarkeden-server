//////////////////////////////////////////////////////////////////////////////
// Filename    : GCShopBought.cpp
// Description :
// When a shop NPC has bought goods from a player, this packet is sent to
// the players talking to the same NPC to tell them that the goods were
// bought.
// On receiving this packet the client has to add the matching item
// to the NPC's list of goods. The shop version is updated too.
//////////////////////////////////////////////////////////////////////////////

#include "GCShopBought.h"

#include "Assert1.h"

//////////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////////
GCShopBought::GCShopBought()

{
    __BEGIN_TRY

    m_ObjectID = 0;
    m_Version = 0;
    m_ShopType = 0;
    m_ShopIndex = 0;
    m_ItemObjectID = 0;
    m_ItemClass = 0;
    m_ItemType = 0;
    m_Durability = 0;
    m_Silver = 0;
    m_Grade = 0;
    m_EnchantLevel = 0;

    __END_CATCH;
}

//////////////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////////////
GCShopBought::~GCShopBought()

{
    __BEGIN_TRY

    __END_CATCH_NO_RETHROW;
}

//////////////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////////////
void GCShopBought::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_Version);
    iStream.read(m_ShopType);
    iStream.read(m_ShopIndex);
    iStream.read(m_ItemObjectID);
    iStream.read(m_ItemClass);
    iStream.read(m_ItemType);

    BYTE optionSize;
    iStream.read(optionSize);

    if (optionSize > kMaxOptionCount)
        throw InvalidProtocolException("too many item options");

    m_OptionType.clear();

    for (int i = 0; i < optionSize; i++) {
        OptionType_t optionType;
        iStream.read(optionType);
        m_OptionType.push_back(optionType);
    }

    iStream.read(m_Durability);
    iStream.read(m_Silver);
    iStream.read(m_Grade);
    iStream.read(m_EnchantLevel);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////////////
void GCShopBought::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_Version);
    oStream.write(m_ShopType);
    oStream.write(m_ShopIndex);
    oStream.write(m_ItemObjectID);
    oStream.write(m_ItemClass);
    oStream.write(m_ItemType);

    if (m_OptionType.size() > kMaxOptionCount)
        throw InvalidProtocolException("too many item options");

    BYTE optionSize = m_OptionType.size();
    oStream.write(optionSize);
    list<OptionType_t>::const_iterator itr = m_OptionType.begin();
    for (; itr != m_OptionType.end(); itr++) {
        OptionType_t optionType = *itr;
        oStream.write(optionType);
    }

    oStream.write(m_Durability);
    oStream.write(m_Silver);
    oStream.write(m_Grade);
    oStream.write(m_EnchantLevel);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCShopBought::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCShopBought(" << "ObjectID:" << (int)m_ObjectID << "," << "ShopVersion: " << (int)m_Version << ","
        << "ShopRackType: " << (int)m_ShopType << "," << "ShopRackIndex: " << (int)m_ShopIndex << ","
        << "ItemObjectID: " << (int)m_ItemObjectID << "," << "ItemClass: " << (int)m_ItemClass << ","
        << "ITemType: " << (int)m_ItemType << "," << "OptionTypeSize: " << (int)m_OptionType.size() << ","
        << "Durability: " << (int)m_Durability << "," << "Silver: " << (int)m_Silver << "," << "Grade: " << (int)m_Grade
        << "," << "EnchantLEvel: " << (int)m_EnchantLevel;
    msg << ")";
    return msg.toString();

    __END_CATCH
}
