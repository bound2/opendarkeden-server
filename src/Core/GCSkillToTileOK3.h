//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToTileOK3.h
// Written By  : elca@ewestsoft.com
// Description : Packet the skill user can see but the target cannot
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SKILL_TO_TILE_OK_3_H__
#define __GC_SKILL_TO_TILE_OK_3_H__


// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCSkillToTileOK3;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCSkillToTileOK3 : public Packet {
public:
    // constructor
    GCSkillToTileOK3();

    // destructor
    ~GCSkillToTileOK3();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SKILL_TO_TILE_OK_3;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    //	PacketSize_t getPacketSize() const  { return szSkillType + szObjectID +(szCoord* 2)
    //			+ szDuration + szBYTE + szObjectID* m_CListNum; }
    PacketSize_t getPacketSize() const {
        return szSkillType + szObjectID + (szCoord * 2) + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCSkillToTileOK3";
    }

    // get packet's debug string
    string toString() const;

    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get / set SkillType
    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

    // get / set Duration
    //	Duration_t getDuration() const  { return m_Duration; }
    //	void setDuration(Duration_t Duration)  { m_Duration = Duration; }

    // get / set X, Y
    Coord_t getX() {
        return m_X;
    }
    void setX(Coord_t X) {
        m_X = X;
    }

    Coord_t getY() {
        return m_Y;
    }
    void setY(Coord_t Y) {
        m_Y = Y;
    }

    // get / set Creature List Number
    //	BYTE getCListNum() const  { return m_CListNum; }

    //	void setCListNum(BYTE CListNum)  { m_CListNum = CListNum; }

    // add / delete  Creature List
    //	void addCListElement(ObjectID_t ObjectID) ;

    // Clear CreatureList
    //	void clearCList()  { m_CList.clear(); m_CListNum = 0; }

    // pop front Element in Status List
    //	ObjectID_t popCListElement()  { ObjectID_t CreatureList = m_CList.front(); m_CList.pop_front(); return
    // CreatureList; }

    BYTE getGrade() const {
        return m_Grade;
    }
    void setGrade(BYTE grade) {
        m_Grade = grade;
    }

private:
    // CEffectID
    ObjectID_t m_ObjectID;

    // SkillType
    SkillType_t m_SkillType;

    // Duration
    //	Duration_t m_Duration;

    // X, Y Position
    Coord_t m_X;

    Coord_t m_Y;

    // CreatureList Element Number
    //	BYTE m_CListNum;

    // Creature List
    //	list<ObjectID_t> m_CList;

    BYTE m_Grade;
};


//////////////////////////////////////////////////////////////////////
//
// class GCSkillToTileOK3Factory;
//
// Factory for GCSkillToTileOK3
//
//////////////////////////////////////////////////////////////////////

class GCSkillToTileOK3Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SKILL_TO_TILE_OK_3;
    static constexpr std::string_view kName = "GCSkillToTileOK3";
    static constexpr PacketSize_t kMaxSize{szSkillType + szObjectID + (szCoord * 2) + szBYTE};

    // constructor
    GCSkillToTileOK3Factory() {}

    // destructor
    virtual ~GCSkillToTileOK3Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCSkillToTileOK3();
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
    //	PacketSize_t getPacketMaxSize() const  { return szSkillType + szObjectID +(szCoord* 2)
    //			+ szDuration + szBYTE + szObjectID + 255; }
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
