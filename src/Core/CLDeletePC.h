//----------------------------------------------------------------------
//
// Filename    : CLDeletePC.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//----------------------------------------------------------------------

#ifndef __CL_DELETE_PC_H__
#define __CL_DELETE_PC_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//----------------------------------------------------------------------
//
// class CLDeletePC;
//
// Packet that deletes the PC in a given slot.
//
//----------------------------------------------------------------------

class CLDeletePC : public Packet {
public:
    CLDeletePC(){};
    virtual ~CLDeletePC(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_DELETE_PC;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Name) + szSlot + de::wire::stringWireSize(m_SSN);
    }

    // get packet's name
    string getPacketName() const {
        return "CLDeletePC";
    }

    // get packet's debug string
    string toString() const;

    // get/set name
    string getName() const {
        return m_Name;
    }
    void setName(string name) {
        m_Name = name;
    }

    // get/set Slot
    Slot getSlot() const {
        return m_Slot;
    }
    void setSlot(Slot slot) {
        m_Slot = slot;
    }

    // get/set SSN
    string getSSN() const {
        return m_SSN;
    }
    void setSSN(const string& SSN) {
        m_SSN = SSN;
    }

private:
    // PC name
    string m_Name;

    // Slot
    Slot m_Slot;

    // Resident registration number
    string m_SSN;
};


//////////////////////////////////////////////////////////////////////
//
// class CLDeletePCFactory;
//
// Factory for CLDeletePC
//
//////////////////////////////////////////////////////////////////////

class CLDeletePCFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_DELETE_PC;
    static constexpr std::string_view kName = "CLDeletePC";
    static constexpr PacketSize_t kMaxSize{szBYTE + 20 + szSlot + szBYTE + 20};

    // create packet
    Packet* createPacket() override {
        return new CLDeletePC();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CLDeletePCHandler;
//
//////////////////////////////////////////////////////////////////////

class CLDeletePCHandler {
public:
    // execute packet's handler
    static void execute(CLDeletePC* pPacket, Player* pPlayer);
};

#endif
