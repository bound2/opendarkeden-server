//////////////////////////////////////////////////////////////////////
//
// Filename    : GCBloodBibleStatus.h
// Written By  : Reiot
//
//////////////////////////////////////////////////////////////////////
//
// STORAGE_CORPSE  ItemType, ZoneID, Race, X, Y  at the start and when moved: load(), returnBloodBible()
// STORAGE_INVENTORY  ItemType, ZoneID, OwnerName, Race, X, Y  when someone picks it up: CGAddZoneToInventory
// STORAGE_MOUSE  ItemType, ZoneID, OwnerName, Race, X, Y  when someone picks it up: CGAddZoneToMouse
// STORAGE_ZONE  ItemType, ZoneID, X, Y  when dropped on the ground: CGAddMouseToZone, CGDissectionCorpse
//
//////////////////////////////////////////////////////////////////////


#ifndef __GC_BLOOD_BIBLE_STATUS_H__
#define __GC_BLOOD_BIBLE_STATUS_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class GCBloodBibleStatus;
//
// Sent when the game server broadcasts one player's BloodBibleStatus to
// the other players. It holds the character name and the BloodBibleStatus string as its data
// fields.
//
//////////////////////////////////////////////////////////////////////

class GCBloodBibleStatus : public Packet {
public:
    GCBloodBibleStatus(){};
    ~GCBloodBibleStatus(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_BLOOD_BIBLE_STATUS;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szItemType + szZoneID + szStorage + de::wire::stringWireSize(m_OwnerName) + szRace + szRace +
               szZoneCoord + szZoneCoord;
    }

    // get packet name
    string getPacketName() const {
        return "GCBloodBibleStatus";
    }

    // get packet's debug string
    string toString() const;

    // get/set text color
    ItemType_t getItemType() const {
        return m_ItemType;
    }
    void setItemType(ItemType_t itemType) {
        m_ItemType = itemType;
    }

    // get/set text color
    ZoneID_t getZoneID() const {
        return m_ZoneID;
    }
    void setZoneID(ZoneID_t zoneID) {
        m_ZoneID = zoneID;
    }

    // get/set text color
    Storage_t getStorage() const {
        return m_Storage;
    }
    void setStorage(Storage_t storage) {
        m_Storage = storage;
    }

    // get/set chatting message
    const string& getOwnerName() const {
        return m_OwnerName;
    }
    void setOwnerName(const string& OwnerName) {
        m_OwnerName = OwnerName;
    }

    // get/set text color
    Race_t getRace() const {
        return m_Race;
    }
    void setRace(Race_t race) {
        m_Race = race;
    }

    // get/set text color
    Race_t getShrineRace() const {
        return m_ShrineRace;
    }
    void setShrineRace(Race_t race) {
        m_ShrineRace = race;
    }

    // get/set text color
    ZoneCoord_t getX() const {
        return m_X;
    }
    void setX(ZoneCoord_t x) {
        m_X = x;
    }

    // get/set text color
    ZoneCoord_t getY() const {
        return m_Y;
    }
    void setY(ZoneCoord_t y) {
        m_Y = y;
    }

private:
    ItemType_t m_ItemType = 0; // Type of the blood bible

    ZoneID_t m_ZoneID = 0;

    Storage_t m_Storage = 0;
    string m_OwnerName;
    Race_t m_Race = 0;
    Race_t m_ShrineRace = 0;
    ZoneCoord_t m_X = 0;
    ZoneCoord_t m_Y = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCBloodBibleStatusFactory;
//
// Factory for GCBloodBibleStatus
//
//////////////////////////////////////////////////////////////////////

class GCBloodBibleStatusFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_BLOOD_BIBLE_STATUS;
    static constexpr std::string_view kName = "GCBloodBibleStatus";
    static constexpr PacketSize_t kMaxSize{szItemType + szZoneID + szStorage + szBYTE + 255 + szRace + szRace +
                                           szZoneCoord + szZoneCoord};

    // create packet
    Packet* createPacket() override {
        return new GCBloodBibleStatus();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCBloodBibleStatusPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
