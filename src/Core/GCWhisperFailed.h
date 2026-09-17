//////////////////////////////////////////////////////////////////////
//
// Filename    : GCWhisperFailed.h
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_WHISPER_FAILED_H__
#define __GC_WHISPER_FAILED_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCWhisperFailed;
//
// Packet used when the game server tells the client that a particular user
// has moved. It carries (CreatureID, X, Y, DIR).
//
//////////////////////////////////////////////////////////////////////

class GCWhisperFailed : public Packet {
public:
    // constructor
    GCWhisperFailed();

    // destructor
    ~GCWhisperFailed();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_WHISPER_FAILED;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "GCWhisperFailed";
    }

    // get packet's debug string
    string toString() const;

private:
};

//////////////////////////////////////////////////////////////////////
//
// class GCWhisperFailedFactory;
//
// Factory for GCWhisperFailed
//
//////////////////////////////////////////////////////////////////////

class GCWhisperFailedFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_WHISPER_FAILED;
    static constexpr std::string_view kName = "GCWhisperFailed";
    static constexpr PacketSize_t kMaxSize{0};

    // constructor
    GCWhisperFailedFactory() {}

    // destructor
    virtual ~GCWhisperFailedFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCWhisperFailed();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
