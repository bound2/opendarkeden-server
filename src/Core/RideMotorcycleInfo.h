//////////////////////////////////////////////////////////////////////////////
// Filename    : RideMotorcycleInfo.h
// Written By  : elca@ewestsoft.com
// Description :
// Packet that carries the information about the motorcycle being ridden.
// Besides the motorcycle object's own information it also carries the
// information about the items in the motorcycle's inventory. For the
// inventory information see RideMotorcycleSlotInfo and InventorySlotInfo.
//////////////////////////////////////////////////////////////////////////////

#ifndef __RIDE_MOTORCYCLE_INFO_H__
#define __RIDE_MOTORCYCLE_INFO_H__

#include <list>

#include "Exception.h"
#include "Packet.h"
#include "RideMotorcycleSlotInfo.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class RideMotorcycleInfo;
//////////////////////////////////////////////////////////////////////////////

class RideMotorcycleInfo {
public:
    RideMotorcycleInfo();
    ~RideMotorcycleInfo() noexcept;

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketSize_t getSize();

    static constexpr uint getMaxSize() {
        return szObjectID +   // motorcycle object id
               szItemType +   // motorcycle type
               szBYTE + 255 + // motorcycle option type
               szBYTE +       // number of item in motorcycle inventory
               RideMotorcycleSlotInfo::getMaxSize() * 60;
    }

    string toString() const;

public:
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }

    void setItemType(ItemType_t ItemType) {
        m_ItemType = ItemType;
    }
    ItemType_t getItemType() const {
        return m_ItemType;
    }

    void addOptionType(OptionType_t OptionType) {
        m_OptionType.push_back(OptionType);
    }
    void setOptionType(const list<OptionType_t>& OptionType) {
        m_OptionType = OptionType;
    }
    int getOptionTypeSize() const {
        return m_OptionType.size();
    }
    const list<OptionType_t>& getOptionType() const {
        return m_OptionType;
    }
    OptionType_t popOptionType() {
        if (m_OptionType.empty())
            return 0;
        OptionType_t optionType = m_OptionType.front();
        m_OptionType.pop_front();
        return optionType;
    }

public:
    BYTE getListNum() const {
        return m_ListNum;
    }

    void addListElement(RideMotorcycleSlotInfo* pRideMotorcycleSlotInfo) {
        m_RideMotorcycleSlotInfoList.push_back(pRideMotorcycleSlotInfo);
        m_ListNum++;
    }

    void clearList() {
        m_RideMotorcycleSlotInfoList.clear();
        m_ListNum = 0;
    }

    RideMotorcycleSlotInfo* popFrontListElement() {
        RideMotorcycleSlotInfo* TempRideMotorcycleSlotInfo = m_RideMotorcycleSlotInfoList.front();
        m_RideMotorcycleSlotInfoList.pop_front();
        return TempRideMotorcycleSlotInfo;
    }

private:
    ObjectID_t m_ObjectID;           // motorcycle object id
    ItemType_t m_ItemType;           // motorcycle item type
    list<OptionType_t> m_OptionType; // motorcycle option type
    BYTE m_ListNum;                  // number of item in motorcycle inventory

    // actual item info in motorcycle inventory
    list<RideMotorcycleSlotInfo*> m_RideMotorcycleSlotInfoList;
};

#endif
