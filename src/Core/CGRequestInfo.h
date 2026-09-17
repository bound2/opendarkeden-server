//--------------------------------------------------------------------------------
//
// Filename    : CGRequestInfo.h
//
//--------------------------------------------------------------------------------

#ifndef __CG_REQUEST_INFO_H__
#define __CG_REQUEST_INFO_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class CGRequestInfo;
//
//--------------------------------------------------------------------------------

class CGRequestInfo : public Packet {
public:
    enum REQUEST_INFO_CODE {
        REQUEST_CHARACTER_INFO,

        REQUEST_INFO_MAX
    };

public:
    CGRequestInfo(){};
    virtual ~CGRequestInfo(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_REQUEST_INFO;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGRequestInfoPacketSize.
    PacketSize_t getPacketSize() const {
        return szBYTE + szuint;
    }

    // get packet name
    string getPacketName() const {
        return "CGRequestInfo";
    }

    // get packet's debug string
    string toString() const;

public:
    // get / set Code
    BYTE getCode() const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

    // get / set Code
    uint getValue() const {
        return m_Value;
    }
    void setValue(uint value) {
        m_Value = value;
    }

private:
    // Code
    BYTE m_Code = 0;
    uint m_Value = 0;
};


//--------------------------------------------------------------------------------
//
// class CGRequestInfoFactory;
//
// Factory for CGRequestInfo
//
//--------------------------------------------------------------------------------

class CGRequestInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_REQUEST_INFO;
    static constexpr std::string_view kName = "CGRequestInfo";
    static constexpr PacketSize_t kMaxSize{szBYTE + szuint};

    // create packet
    Packet* createPacket() override {
        return new CGRequestInfo();
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
    // Define and return const static CGRequestInfoPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
// class CGRequestInfoHandler;
//
//--------------------------------------------------------------------------------

class CGRequestInfoHandler {
public:
    // execute packet's handler
    static void execute(CGRequestInfo* pPacket, Player* player);
};

#endif
