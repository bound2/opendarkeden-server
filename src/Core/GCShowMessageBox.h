//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowMessageBox.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SHOW_MESSAGE_BOX_H__
#define __GC_SHOW_MESSAGE_BOX_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"


//////////////////////////////////////////////////////////////////////
//
// class GCShowMessageBox;
//
// Make the client open the guild registration window.
//
//////////////////////////////////////////////////////////////////////

class GCShowMessageBox : public Packet {
public:
    GCShowMessageBox(){};
    ~GCShowMessageBox(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOW_MESSAGE_BOX;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Message);
    }

    // get packet name
    string getPacketName() const {
        return "GCShowMessageBox";
    }

    // get packet's debug string
    string toString() const;

    // get/set Message
    string getMessage() const {
        return m_Message;
    }
    void setMessage(const string& message) {
        m_Message = message;
    }


private:
    // Message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class GCShowMessageBoxFactory;
//
// Factory for GCShowMessageBox
//
//////////////////////////////////////////////////////////////////////

class GCShowMessageBoxFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOW_MESSAGE_BOX;
    static constexpr std::string_view kName = "GCShowMessageBox";
    static constexpr PacketSize_t kMaxSize{szBYTE + 256};

    // create packet
    Packet* createPacket() override {
        return new GCShowMessageBox();
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
    // Define and return const static GCSystemMessagePacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GCShowMessageBox;
//
//////////////////////////////////////////////////////////////////////

#endif
