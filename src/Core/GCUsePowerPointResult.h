//--------------------------------------------------------------------------------
//
// Filename    : GCUsePowerPointResult.h
// Written By  : bezz
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GC_USE_POWER_POINT_RESULT_H__
#define __GC_USE_POWER_POINT_RESULT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCUsePowerPointResult;
//
//--------------------------------------------------------------------------------

class GCUsePowerPointResult : public Packet {
public:
    enum RESULT_CODE {
        NO_ERROR = 0,
        NOT_ENOUGH_POWER_POINT,     // Not enough power points.
        NOT_ENOUGH_INVENTORY_SPACE, // There is not enough room in the inventory.
    };

    enum ITEM_CODE {
        CANDY = 0,            // One candy
        RESURRECTION_SCROLL,  // One resurrection scroll
        ELIXIR_SCROLL,        // One elixir scroll
        MEGAPHONE,            // Thirty minutes of the megaphone
        NAMING_PEN,           // One naming pen
        SIGNPOST,             // Six hours of the notice board
        BLACK_RICE_CAKE_SOUP, // One black rice cake soup
    };

    // The last code of each list. read() refuses a byte past it, since no
    // branch of the receiver handles one.
    static const BYTE kLastResultCode = NOT_ENOUGH_INVENTORY_SPACE;
    static const BYTE kLastItemCode = BLACK_RICE_CAKE_SOUP;

public:
    GCUsePowerPointResult();
    ~GCUsePowerPointResult();

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_USE_POWER_POINT_RESULT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + szBYTE + szint;
    }

    // get packet name
    string getPacketName() const {
        return "GCUsePowerPointResult";
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

    // get / set Item Code
    BYTE getItemCode() const {
        return m_ItemCode;
    }
    void setItemCode(BYTE itemcode) {
        m_ItemCode = itemcode;
    }

    // get / set Power Point
    int getPowerPoint() const {
        return m_PowerPoint;
    }
    void setPowerPoint(int powerpoint) {
        m_PowerPoint = powerpoint;
    }

    //--------------------------------------------------
    // data members
    //--------------------------------------------------
private:
    // Error code
    BYTE m_ErrorCode;

    // Item code
    BYTE m_ItemCode;

    // Power points
    int m_PowerPoint = 0;
};


//--------------------------------------------------------------------------------
//
// class GCUsePowerPointResultFactory;
//
// Factory for GCUsePowerPointResult
//
//--------------------------------------------------------------------------------

class GCUsePowerPointResultFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_USE_POWER_POINT_RESULT;
    static constexpr std::string_view kName = "GCUsePowerPointResult";
    static constexpr PacketSize_t kMaxSize{szBYTE + szBYTE + szint};

    // create packet
    Packet* createPacket() override {
        return new GCUsePowerPointResult();
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
    // Define and return const static GCUsePowerPointResultPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
