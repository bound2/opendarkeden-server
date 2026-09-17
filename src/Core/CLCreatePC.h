//----------------------------------------------------------------------
//
// Filename    : CLCreatePC.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __CL_CREATE_PC_H__
#define __CL_CREATE_PC_H__

// include files
#include <bitset>

#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//----------------------------------------------------------------------
//
// class CLCreatePC;
//
// When a new slayer character is created, its information is put in this packet and sent to the server.
//
//----------------------------------------------------------------------

class CLCreatePC : public Packet {
public:
    enum { SLAYER_BIT_SEX, SLAYER_BIT_HAIRSTYLE, SLAYER_BIT_HAIRSTYLE2, SLAYER_BIT_MAX };

    enum {
        SLAYER_COLOR_HAIR,
        SLAYER_COLOR_SKIN,
        SLAYER_COLOR_SHIRT,
        SLAYER_COLOR_SHIRT2,
        SLAYER_COLOR_JEANS,
        SLAYER_COLOR_JEANS2,
        SLAYER_COLOR_MAX
    };


public:
    CLCreatePC(){};
    virtual ~CLCreatePC(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_CREATE_PC;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CLCreatePCPacketSize.
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Name)          // name
               + szSlot                                  // slot
               + szBYTE                                  // slayer flags (3 bits)
               + szAttr * 3 + szColor * SLAYER_COLOR_MAX // attributes and colours
               + szRace;                                 // race
    }

    // get packet's name
    string getPacketName() const {
        return "CLCreatePC";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set name
    string getName() const {
        return m_Name;
    }
    void setName(string name) {
        m_Name = name;
    }

    // get/set slot
    Slot getSlot() const {
        return m_Slot;
    }
    void setSlot(Slot slot) {
        m_Slot = slot;
    }

    // get/set sex
    Sex getSex() const {
        return m_BitSet.test(SLAYER_BIT_SEX) ? MALE : FEMALE;
    }
    void setSex(Sex sex) {
        m_BitSet.set(SLAYER_BIT_SEX, (sex == MALE ? true : false));
    }

    // get/set hair style
    HairStyle getHairStyle() const {
        return HairStyle((m_BitSet.to_ulong() >> 1) & 3);
    }
    void setHairStyle(HairStyle hairStyle) {
        m_BitSet |= bitset<SLAYER_BIT_MAX>(hairStyle << 1);
    }

    // get/set race. by sigi. 2002.10.31
    // bool isSlayer() const  { return ((m_BitSet.to_ulong() >> 3) & 1)==0; }
    // void setSlayer(bool bSlayer=true)  { m_BitSet |= bitset<SLAYER_BIT_MAX>((int)(bSlayer==false) << 3); }

    // get/set hair color
    Color_t getHairColor() const {
        return m_Colors[SLAYER_COLOR_HAIR];
    }
    void setHairColor(Color_t hairColor) {
        m_Colors[SLAYER_COLOR_HAIR] = hairColor;
    }

    // get/set skin color
    Color_t getSkinColor() const {
        return m_Colors[SLAYER_COLOR_SKIN];
    }
    void setSkinColor(Color_t skinColor) {
        m_Colors[SLAYER_COLOR_SKIN] = skinColor;
    }

    // get/set shirt color
    Color_t getShirtColor(ColorType colorType = MAIN_COLOR) const {
        return m_Colors[SLAYER_COLOR_SHIRT + (uint)colorType];
    }
    void setShirtColor(Color_t shirtColor, ColorType colorType = MAIN_COLOR) {
        m_Colors[SLAYER_COLOR_SHIRT + (uint)colorType] = shirtColor;
    }

    // get/set jeans color
    Color_t getJeansColor(ColorType colorType = MAIN_COLOR) const {
        return m_Colors[SLAYER_COLOR_JEANS + (uint)colorType];
    }
    void setJeansColor(Color_t jeansColor, ColorType colorType = MAIN_COLOR) {
        m_Colors[SLAYER_COLOR_JEANS + (uint)colorType] = jeansColor;
    }

    // get/set STR
    Attr_t getSTR() const {
        return m_STR;
    }
    void setSTR(Attr_t str) {
        m_STR = str;
    }
    // get/set DEX
    Attr_t getDEX() const {
        return m_DEX;
    }
    void setDEX(Attr_t dex) {
        m_DEX = dex;
    }
    // get/set INT
    Attr_t getINT() const {
        return m_INT;
    }
    void setINT(Attr_t inte) {
        m_INT = inte;
    }

    // get/set Race
    Race_t getRace() const {
        return m_Race;
    }
    void setRace(Race_t race) {
        m_Race = race;
    }


private:
    // Name of the PC
    string m_Name;

    // Slot
    Slot m_Slot;

    // Slayer flags
    bitset<SLAYER_BIT_MAX> m_BitSet;

    // Slayer colour information
    Color_t m_Colors[SLAYER_COLOR_MAX];

    // STR, DEX, INTE
    Attr_t m_STR;
    Attr_t m_DEX;
    Attr_t m_INT;

    // Race
    Race_t m_Race;
};


//////////////////////////////////////////////////////////////////////
//
// class CLCreatePCFactory;
//
// Factory for CLCreatePC
//
//////////////////////////////////////////////////////////////////////

class CLCreatePCFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_CREATE_PC;
    static constexpr std::string_view kName = "CLCreatePC";
    static constexpr PacketSize_t kMaxSize{szBYTE + 20                                           // Name
                                           + szSlot                                              // Slot
                                           + szBYTE                                              // Slayer flags (3 bit)
                                           + szAttr * 3 + szColor * CLCreatePC::SLAYER_COLOR_MAX // Colour information
                                           + szRace};                                            // Race

    // create packet
    Packet* createPacket() override {
        return new CLCreatePC();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CLCreatePCPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CLCreatePCHandler;
//
//////////////////////////////////////////////////////////////////////

class CLCreatePCHandler {
public:
    // execute packet's handler
    static void execute(CLCreatePC* pPacket, Player* pPlayer);
};

#endif
