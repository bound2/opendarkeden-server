//--------------------------------------------------------------------------------
//
// Filename    : GCShopVersion.cpp
// Description : Packet that tells the player the server-side shop version.
//
//--------------------------------------------------------------------------------

// include files
#include "GCShopVersion.h"

#include "Assert1.h"


//--------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------

GCShopVersion::GCShopVersion()

{
    __BEGIN_TRY

    __END_CATCH;
}

//--------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------
GCShopVersion::~GCShopVersion()

{
    __BEGIN_TRY

    __END_CATCH_NO_RETHROW;
}

//--------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------
void GCShopVersion::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // read object id
    iStream.read(m_ObjectID);

    // read versions
    for (ShopRackType_t i = 0; i < SHOP_RACK_TYPE_MAX; i++)
        iStream.read(m_Version[i]);

    iStream.read(m_MarketCondSell);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCShopVersion::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // write object id
    oStream.write(m_ObjectID);

    // write versions
    for (ShopRackType_t i = 0; i < SHOP_RACK_TYPE_MAX; i++)
        oStream.write(m_Version[i]);

    oStream.write(m_MarketCondSell);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCShopVersion::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCShopVersion(" << "ObjectID:" << m_ObjectID;

    for (ShopRackType_t i = 0; i < SHOP_RACK_TYPE_MAX; i++)
        msg << " Version[" << (int)i << "] : " << (int)m_Version[i];

    msg << ")";

    return msg.toString();

    __END_CATCH
}
