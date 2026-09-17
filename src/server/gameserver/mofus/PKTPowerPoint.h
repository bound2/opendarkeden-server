/////////////////////////////////////////////////////////////////////////////
// Filename : PKTPowerPoint.h
// Desc		: hands the online company the value drawn from the PowerRing DB
// 			  using the data sent from the online game.
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_POWERPOINT_H__
#define __PKT_POWERPOINT_H__

// include files
#include "Assert.h"
#include "MPacket.h"

// packet layout
struct _PKT_POWERPOINT {
    int nSize;
    int nCode;
    int nMoDataCode;         // Index for handling the data, a Unique value
    char sPhoneNo[12];       // the mobile number of the user that registered the PowerRing points
    char sMemID[20];         // the PowerRing user account ID
    int nMatchingCode;       // the matching code inside the PowerRing table
    char sMoGameName[20];    // the mobile game name tied to the online game
    int nMoGameCode;         // the mobile game code tied to the online game
    int nOnGameCode;         // online game code
    char sOnGameName[20];    // online game name
    int nOnGameSerCode;      // online game server code
    char sOnGameSerName[20]; // online game server name
    char sOnGameID[20];      // online game id
    char sCharName[40];      // online game character name
    int nOnAbilityCode;      // the stat code applied in the online game
    char sOnAbilityName[20]; // the stat name applied in the online game
    int nPowerPoint;         // the PowerRing points the user sent
    char sInputDate[20];     // when the user entered the PowerRing points on mobile
    int nIndex;              // an index for the online company's convenience
    int nContinue;           // whether more data follows. 1:yes. 0:no
};

const int szPKTPowerPoint = sizeof(_PKT_POWERPOINT);

// class PKTPowerPoint
class PKTPowerPoint : public _PKT_POWERPOINT, public MPacket {
public:
    // constructor
    PKTPowerPoint();

public:
    // Returns the packet id.
    MPacketID_t getID() const;

    // Returns the packet's size.
    MPacketSize_t getSize() const {
        return szPKTPowerPoint - szMPacketSize;
    }

    // Creates a new packet and returns it
    MPacket* create() {
        MPacket* pPacket = new PKTPowerPoint;
        Assert(pPacket != NULL);
        return pPacket;
    }

    // Reads data from the input stream and initialises the packet.
    void read(SocketInputStream& iStream);

    // Sends the packet's binary image to the output stream.
    void write(SocketOutputStream& oStream);

    // debug message
    string toString() const;

public:
    // get methods
    bool isContinue() const {
        return nContinue == 1;
    }

    // get PowerPoint
    int getPowerPoint() const {
        return nPowerPoint;
    }

    // get GameCode
    int getGameCode() const {
        return nOnGameCode;
    }

    // get GameServerCode
    int getGameServerCode() const {
        return nOnGameSerCode;
    }

    // get Character Name
    const char* getCharacterName() const {
        return (const char*)sCharName;
    }
};

#endif
