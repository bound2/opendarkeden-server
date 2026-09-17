//--------------------------------------------------------------------------------
//
// Filename    : GCShopSellFail.cpp
// Description : The player tried to sell goods to an NPC and, for some
//               reason it failed, so this packet is sent to the player.
//
//--------------------------------------------------------------------------------

// include files
#include "GCShopSellFail.h"

#include "Assert1.h"


//--------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------

GCShopSellFail::GCShopSellFail()

{
    __BEGIN_TRY

    __END_CATCH;
}

//--------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------
GCShopSellFail::~GCShopSellFail()

{
    __BEGIN_TRY


    __END_CATCH_NO_RETHROW;
}

//--------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------
void GCShopSellFail::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    //-----------------------------------------------------------------
    // read object id
    //-----------------------------------------------------------------
    iStream.read(m_ObjectID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCShopSellFail::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    //-----------------------------------------------------------------
    // write object id
    //-----------------------------------------------------------------
    oStream.write(m_ObjectID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCShopSellFail::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCShopSellFail(" << "ObjectID:" << m_ObjectID << ")";
    return msg.toString();

    __END_CATCH
}
