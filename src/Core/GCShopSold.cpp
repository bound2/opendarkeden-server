//--------------------------------------------------------------------------------
//
// Filename    : GCShopSold.cpp
// Description : One player bought goods from a shop NPC, and
//               another player was also talking to the same shop NPC,
//               that player's item list has to be resynchronised.
//               This packet is the one for that.
//
//--------------------------------------------------------------------------------

// include files
#include "GCShopSold.h"

#include "Assert1.h"

//--------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------

GCShopSold::GCShopSold()

{
    __BEGIN_TRY

    __END_CATCH;
}

//--------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------
GCShopSold::~GCShopSold()

{
    __BEGIN_TRY

    __END_CATCH_NO_RETHROW;
}

//--------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------
void GCShopSold::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_Version);
    iStream.read(m_RackType);
    iStream.read(m_RackIndex);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCShopSold::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_Version);
    oStream.write(m_RackType);
    oStream.write(m_RackIndex);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCShopSold::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCShopSold(" << "ObjectID:" << (int)m_ObjectID << ",ShopVersion: " << (int)m_Version
        << ",ShopRackType: " << (int)m_RackType << ",ShopRackIndex: " << (int)m_RackIndex << ")";
    return msg.toString();

    __END_CATCH
}
