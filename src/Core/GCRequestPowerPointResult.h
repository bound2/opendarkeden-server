//--------------------------------------------------------------------------------
//
// Filename    : GCRequestPowerPointResult.h
// Written By  : bezz
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GC_REQUEST_POWER_POINT_RESULT_H__
#define __GC_REQUEST_POWER_POINT_RESULT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCRequestPowerPointResult;
//
//--------------------------------------------------------------------------------

class GCRequestPowerPointResult : public Packet {
public:
    enum RESULT_CODE {
        NO_ERROR = 0,
        SERVER_ERROR,  // The PowerZzang server is alive but is not working properly at the moment
        PROCESS_ERROR, // Server processing error (e.g. a DB error)
        NO_MEMBER,     // When the user is not a PowerZzang member
        NO_POINT,      // No PowerZzang points accumulated
        NO_MATCHING,   // No matching information.
                       // When the game was not matched on the PowerZzang home page
                       // Hand out the sentence that leads to matching on the PowerZzang home page.
        CONNECT_ERROR, // Game code check failure and packet error
    };

public:
    GCRequestPowerPointResult();
    ~GCRequestPowerPointResult();

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_REQUEST_POWER_POINT_RESULT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + szint + szint;
    }

    // get packet name
    string getPacketName() const {
        return "GCRequestPowerPointResult";
    }

    // get packet's debug string
    string toString() const;

    // get / set Error Code
    BYTE getErrorCode() const {
        return m_ErrorCode;
    }
    void setErrorCode(BYTE errorcode) {
        m_ErrorCode = errorcode;
    }

    // get / set SumPowerPoint
    int getSumPowerPoint() const {
        return m_SumPowerPoint;
    }
    void setSumPowerPoint(int powerpoint) {
        m_SumPowerPoint = powerpoint;
    }

    // get / set RequestPowerPoint
    int getRequestPowerPoint() const {
        return m_RequestPowerPoint;
    }
    void setRequestPowerPoint(int powerpoint) {
        m_RequestPowerPoint = powerpoint;
    }

    //--------------------------------------------------
    // data members
    //--------------------------------------------------
private:
    // Error code
    BYTE m_ErrorCode;

    // PowerZzang points accumulated so far
    int m_SumPowerPoint;

    // PowerZzang points fetched by the request
    int m_RequestPowerPoint;
};


//--------------------------------------------------------------------------------
//
// class GCRequestPowerPointResultFactory;
//
// Factory for GCRequestPowerPointResult
//
//--------------------------------------------------------------------------------

class GCRequestPowerPointResultFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REQUEST_POWER_POINT_RESULT;
    static constexpr std::string_view kName = "GCRequestPowerPointResult";
    static constexpr PacketSize_t kMaxSize{szBYTE + szint + szint};

    // create packet
    Packet* createPacket() override {
        return new GCRequestPowerPointResult();
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
    // Define and return const static GCRequestPowerPointResultPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
