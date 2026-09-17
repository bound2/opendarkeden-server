/////////////////////////////////////////////////////////////////////////////
// Filename : PKTUserInfo.h
// Desc		: sends the online game user ID, character name, member name and
// 			  server information to the PowerRing server to authenticate the member.
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_USERINFO_H__
#define __PKT_USERINFO_H__

// include files
#include "Assert.h"
#include "MPacket.h"

// packet layout
struct _PKT_USERINFO {
    int nSize;
    int nCode;
    char sJuminNo[20];   // national id number
    char sHandPhone[12]; // mobile phone number
    int nIndex;          // an index for the online company's convenience
};

const int szPKTUserInfo = sizeof(_PKT_USERINFO);

// class PKTUserInfo
class PKTUserInfo : public _PKT_USERINFO, public MPacket {
public:
    // constructor
    PKTUserInfo();

public:
    // Returns the packet id.
    MPacketID_t getID() const;

    // Returns the packet's size.
    MPacketSize_t getSize() const {
        return szPKTUserInfo - szMPacketSize;
    }

    // Creates a new packet and returns it
    MPacket* create() {
        MPacket* pPacket = new PKTUserInfo;
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
    // set ssn & cellnum
    void setSSN(const string& ssn);
    void setCellNum(const string& cellnum);
};

#endif
