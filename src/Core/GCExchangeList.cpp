//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExchangeList.cpp
// Written By  : Exchange System
// Description : Server sends listing list to client
//////////////////////////////////////////////////////////////////////////////

#include "GCExchangeList.h"

#include <string>

using namespace std;

// Out-of-line definitions so the constants may also be odr-used (bound to a
// reference, e.g. std::min) by handlers and tests, not only read as
// compile-time constants.
const PacketSize_t GCExchangeList::kMaxListingString;
const PacketSize_t GCExchangeList::kMaxListingsPerPage;

// Stub execute() for server side (GC packets don't execute on server)
void GCExchangeList::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    // Read page info
    iStream.read(m_Page);
    iStream.read(m_PageSize);
    iStream.read(m_Total);

    // Read listing count
    uint16_t count;
    iStream.read(count);

    // A count above the page bound cannot have been produced by write() (the
    // handler clamps the page size to it, which is what makes getPacketMaxSize()
    // a real bound), and honouring one would consume far past this packet into
    // whatever is queued behind it. The client's counterpart refuses it here
    // too; keep the two in step.
    if (count > kMaxListingsPerPage)
        throw InvalidProtocolException("GCExchangeList: more listings than a page can hold");

    // Drop whatever a previous read left behind. read() must fully overwrite
    // the packet's state to be a true mirror of write(); appending to the old
    // contents would make a recycled packet object report more listings than
    // the wire carried. The client's counterpart clears here too.
    m_Listings.clear();

    // Read listings — this must stay an exact mirror of write() below, both in
    // field order and in the clamping of every length-prefixed string. Each
    // string's else-branch is part of that mirror: a zero length on the wire
    // must leave an empty string, not the value the listing happened to hold.
    for (uint16_t i = 0; i < count; i++) {
        ExchangeListing listing;

        // char buf[256] is safe for any uint8_t len: at len == 255 the
        // terminator lands on buf[255], the last element.
        char buf[256];
        uint8_t len;

        uint64_t listingID;
        iStream.read(listingID);
        listing.listingID = (int64_t)listingID;

        uint16_t serverID;
        iStream.read(serverID);
        listing.serverID = (int16_t)serverID;

        // SellerAccount
        iStream.read(len);
        if (len > 0) {
            iStream.read(buf, len);
            buf[len] = '\0';
            listing.sellerAccount = buf;
        } else {
            listing.sellerAccount.clear();
        }

        // SellerPlayer
        iStream.read(len);
        if (len > 0) {
            iStream.read(buf, len);
            buf[len] = '\0';
            listing.sellerPlayer = buf;
        } else {
            listing.sellerPlayer.clear();
        }

        iStream.read(listing.sellerRace);
        iStream.read(listing.itemClass);
        iStream.read(listing.itemType);

        uint64_t itemID;
        iStream.read(itemID);
        listing.itemID = (int64_t)itemID;

        iStream.read(listing.objectID);
        iStream.read(listing.pricePoint);
        iStream.read(listing.currency);
        iStream.read(listing.status);

        // BuyerAccount
        iStream.read(len);
        if (len > 0) {
            iStream.read(buf, len);
            buf[len] = '\0';
            listing.buyerAccount = buf;
        } else {
            listing.buyerAccount.clear();
        }

        // BuyerPlayer
        iStream.read(len);
        if (len > 0) {
            iStream.read(buf, len);
            buf[len] = '\0';
            listing.buyerPlayer = buf;
        } else {
            listing.buyerPlayer.clear();
        }

        iStream.read(listing.taxRate);
        iStream.read(listing.taxAmount);

        // Timestamp strings (soldAt/cancelledAt/updatedAt are not on the wire)
        iStream.read(len);
        if (len > 0) {
            iStream.read(buf, len);
            buf[len] = '\0';
            listing.createdAt = buf;
        } else {
            listing.createdAt.clear();
        }

        iStream.read(len);
        if (len > 0) {
            iStream.read(buf, len);
            buf[len] = '\0';
            listing.expireAt = buf;
        } else {
            listing.expireAt.clear();
        }

        iStream.read(listing.version);

        // Snapshot fields
        // ItemName
        iStream.read(len);
        if (len > 0) {
            iStream.read(buf, len);
            buf[len] = '\0';
            listing.itemName = buf;
        } else {
            listing.itemName.clear();
        }

        iStream.read(listing.enchantLevel);
        iStream.read(listing.grade);
        iStream.read(listing.durability);
        iStream.read(listing.silver);
        iStream.read(listing.optionType1);
        iStream.read(listing.optionType2);
        iStream.read(listing.optionType3);
        iStream.read(listing.optionValue1);
        iStream.read(listing.optionValue2);
        iStream.read(listing.optionValue3);
        iStream.read(listing.stackCount);

        m_Listings.push_back(listing);
    }

    __END_CATCH
}

void GCExchangeList::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // Write page info
    oStream.write(m_Page);
    oStream.write(m_PageSize);
    oStream.write(m_Total);

    // Write listing count
    uint16_t count = (uint16_t)m_Listings.size();
    oStream.write(count);

    // Write listings.
    //
    // Every string below is clamped to kMaxListingString, and getPacketSize()
    // repeats the identical clamp field for field. SocketOutputStream::
    // writePacket() emits getPacketSize() into the stream header BEFORE it
    // calls write(), so the two must agree for every possible value. A BYTE
    // length prefix cannot carry more than 255 anyway, so an over-long string
    // has to lose bytes; the only question is whether both sides lose the same
    // ones. Clamping in only one place (or throwing from write(), with the
    // header already on the wire) desynchronises the stream for every
    // subsequent packet on the connection.
    for (const auto& listing : m_Listings) {
        // Write basic fields
        oStream.write((uint64_t)listing.listingID);
        oStream.write((uint16_t)listing.serverID);

        // Write strings with length prefix
        // SellerAccount
        uint8_t len = (uint8_t)(listing.sellerAccount.length() > kMaxListingString ? kMaxListingString
                                                                                   : listing.sellerAccount.length());
        oStream.write(len);
        if (len > 0)
            oStream.write(listing.sellerAccount.c_str(), len);

        // SellerPlayer
        len = (uint8_t)(listing.sellerPlayer.length() > kMaxListingString ? kMaxListingString
                                                                          : listing.sellerPlayer.length());
        oStream.write(len);
        if (len > 0)
            oStream.write(listing.sellerPlayer.c_str(), len);

        oStream.write(listing.sellerRace);
        oStream.write(listing.itemClass);
        oStream.write(listing.itemType);
        oStream.write((uint64_t)listing.itemID);
        oStream.write(listing.objectID);
        oStream.write(listing.pricePoint);
        oStream.write(listing.currency);
        oStream.write(listing.status);

        // BuyerAccount
        len = (uint8_t)(listing.buyerAccount.length() > kMaxListingString ? kMaxListingString
                                                                          : listing.buyerAccount.length());
        oStream.write(len);
        if (len > 0)
            oStream.write(listing.buyerAccount.c_str(), len);

        // BuyerPlayer
        len = (uint8_t)(listing.buyerPlayer.length() > kMaxListingString ? kMaxListingString
                                                                         : listing.buyerPlayer.length());
        oStream.write(len);
        if (len > 0)
            oStream.write(listing.buyerPlayer.c_str(), len);

        oStream.write(listing.taxRate);
        oStream.write(listing.taxAmount);

        // Timestamp strings
        len =
            (uint8_t)(listing.createdAt.length() > kMaxListingString ? kMaxListingString : listing.createdAt.length());
        oStream.write(len);
        if (len > 0)
            oStream.write(listing.createdAt.c_str(), len);

        len = (uint8_t)(listing.expireAt.length() > kMaxListingString ? kMaxListingString : listing.expireAt.length());
        oStream.write(len);
        if (len > 0)
            oStream.write(listing.expireAt.c_str(), len);

        oStream.write(listing.version);

        // Snapshot fields
        len = (uint8_t)(listing.itemName.length() > kMaxListingString ? kMaxListingString : listing.itemName.length());
        oStream.write(len);
        if (len > 0)
            oStream.write(listing.itemName.c_str(), len);

        oStream.write(listing.enchantLevel);
        oStream.write(listing.grade);
        oStream.write(listing.durability);
        oStream.write(listing.silver);
        oStream.write(listing.optionType1);
        oStream.write(listing.optionType2);
        oStream.write(listing.optionType3);
        oStream.write(listing.optionValue1);
        oStream.write(listing.optionValue2);
        oStream.write(listing.optionValue3);
        oStream.write(listing.stackCount);
    }

    __END_CATCH
}

PacketSize_t GCExchangeList::getPacketSize() const {
    PacketSize_t size = 0;
    size += sizeof(m_Page);
    size += sizeof(m_PageSize);
    size += sizeof(m_Total);
    size += sizeof(uint16_t); // count

    for (const auto& listing : m_Listings) {
        size += sizeof(uint64_t); // listingID
        size += sizeof(uint16_t); // serverID
        // Same clamps as write(), so getPacketSize() == bytes written for ANY value.
        size += szBYTE + (listing.sellerAccount.length() > kMaxListingString ? kMaxListingString
                                                                             : listing.sellerAccount.length());
        size += szBYTE +
                (listing.sellerPlayer.length() > kMaxListingString ? kMaxListingString : listing.sellerPlayer.length());
        size += sizeof(listing.sellerRace);
        size += sizeof(listing.itemClass);
        size += sizeof(listing.itemType);
        size += sizeof(uint64_t); // itemID
        size += sizeof(listing.objectID);
        size += sizeof(listing.pricePoint);
        size += sizeof(listing.currency);
        size += sizeof(listing.status);
        size += szBYTE +
                (listing.buyerAccount.length() > kMaxListingString ? kMaxListingString : listing.buyerAccount.length());
        size += szBYTE +
                (listing.buyerPlayer.length() > kMaxListingString ? kMaxListingString : listing.buyerPlayer.length());
        size += sizeof(listing.taxRate);
        size += sizeof(listing.taxAmount);
        size +=
            szBYTE + (listing.createdAt.length() > kMaxListingString ? kMaxListingString : listing.createdAt.length());
        size +=
            szBYTE + (listing.expireAt.length() > kMaxListingString ? kMaxListingString : listing.expireAt.length());
        size += sizeof(listing.version);
        size +=
            szBYTE + (listing.itemName.length() > kMaxListingString ? kMaxListingString : listing.itemName.length());
        size += sizeof(listing.enchantLevel);
        size += sizeof(listing.grade);
        size += sizeof(listing.durability);
        size += sizeof(listing.silver);
        size += sizeof(listing.optionType1);
        size += sizeof(listing.optionType2);
        size += sizeof(listing.optionType3);
        size += sizeof(listing.optionValue1);
        size += sizeof(listing.optionValue2);
        size += sizeof(listing.optionValue3);
        size += sizeof(listing.stackCount);
    }

    return size;
}

string GCExchangeList::toString() const {
    StringStream msg;
    msg << "GCExchangeList(" << "Page:" << m_Page << ",PageSize:" << m_PageSize << ",Total:" << m_Total
        << ",Count:" << m_Listings.size() << ")";
    return msg.toString();
}

void GCExchangeList::setListings(const vector<ExchangeListing>& listings) {
    m_Listings = listings;
}
