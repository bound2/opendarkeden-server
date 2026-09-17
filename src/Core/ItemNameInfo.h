//--------------------------------------------------------------------------------
//
// Filename    : ItemNameInfo.h
// Written By  :
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __ITEM_NAME_H__
#define __ITEM_NAME_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//--------------------------------------------------------------------------------
//
// class ItemNameInfo;
//
//--------------------------------------------------------------------------------

class ItemNameInfo {
public:
    ItemNameInfo() {}
    ItemNameInfo(ObjectID_t objectID, string name) {
        m_ObjectID = objectID;
        m_Name = name;
    }
    virtual ~ItemNameInfo() {}

    // Read data from the input stream (buffer) and initialise the packet.
    virtual void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    virtual void write(SocketOutputStream& oStream) const;

    virtual PacketSize_t getSize() const;

    static constexpr PacketSize_t getMaxSize() {
        return (szObjectID + szBYTE + (szBYTE * 20));
    }

    // get packet's debug string
    string toString() const;

public:
    void setObjectID(ObjectID_t objectID) {
        m_ObjectID = objectID;
    }
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }

    void setName(string name) {
        m_Name = name;
    }
    string getName() const {
        return m_Name;
    }

protected:
    ObjectID_t m_ObjectID;
    string m_Name;
};

#endif
