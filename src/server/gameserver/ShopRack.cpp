//////////////////////////////////////////////////////////////////////
// Filename    : ShopRack.cpp
// Description :
//////////////////////////////////////////////////////////////////////

#include "ShopRack.h"

#include "Assert.h"

//////////////////////////////////////////////////////////////////////
// constructor & destructor
//////////////////////////////////////////////////////////////////////

ShopRack::ShopRack() : ItemRack(SHOP_RACK_INDEX_MAX) {
    // Initialize the version.
    // If the version started at 0, a client that also starts at 0 would match
    // versions and never ask for the shop's item list, even when it does not
    // actually hold one.
    // To prevent that, it starts at 100 instead.
    m_Version = 100;
}

ShopRack::~ShopRack() {}
