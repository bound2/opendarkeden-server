/////////////////////////////////////////////////////////////////////////////
// filename	: MPacketID.h
/////////////////////////////////////////////////////////////////////////////

#ifndef __MPACKET_ID_H__
#define __MPACKET_ID_H__

// MPacketID send enum
enum {
    PTC_CONNECT_ASK = 0x01, // the game server asks to connect
    PTC_LOGOUT = 0x11,      // the game server closes the connection
    PTC_USERINFO = 0x20,    // asks for the user information
    PTC_RECEIVE_OK = 0x30,  // whether the data received was handled
    PTC_RESULT = 0x40,      // whether the last data received was handled
    PTC_ERROR = 0xFF,       // used by the game server to report a handling error

    PTC_SEND_MAX
};

// MPacketID recv enum
enum {
    PTS_CONNECT_ACCEPT = 0x01, // the game server's connection request is granted
    PTS_POWERPOINT = 0x20,     // the information that was asked for
    PTS_ERROR = 0xFF,          // server error notice

    PTC_MAX
};

#endif
