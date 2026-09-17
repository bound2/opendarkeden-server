/////////////////////////////////////////////////////////////////////////////
// Filename : PKTPowerPoint.cpp
// Desc		: hands the online company the value drawn from the PowerRing DB
// 			  using the data sent from the online game.
/////////////////////////////////////////////////////////////////////////////

// include files
#include "PKTPowerPoint.h"

#include "MPacketID.h"

// constructor
PKTPowerPoint::PKTPowerPoint() {
    // memset( this, 0, szPKTPowerPoint );

    nSize = szPKTPowerPoint - szMPacketSize;
}

// Reads data from the input stream and initialises the packet.
void PKTPowerPoint::read(SocketInputStream& iStream) {
    iStream.read((char*)this, szPKTPowerPoint);

    // change order - network to host
    //	nSize				= ntohl( nSize );
    //	nCode				= ntohl( nCode );
    //	nMoDataCode			= ntohl( nMoDataCode );
    //	nMatchingCode		= ntohl( nMatchingCode );
    //	nMoGameCode			= ntohl( nMoGameCode );
    //	nOnGameCode			= ntohl( nOnGameCode );
    //	nOnGameSerCode		= ntohl( nOnGameSerCode );
    //	nOnAbilityCode		= ntohl( nOnAbilityCode );
    //	nPowerPoint			= ntohl( nPowerPoint );
    //	nContinue			= ntohl( nContinue );
    //	nIndex				= ntohl( nIndex );
}

// Sends the packet's binary image to the output stream.
void PKTPowerPoint::write(SocketOutputStream& oStream) {
    nCode = getID();

    // change order - host to network
    //	nSize				= htonl( nSize );
    //	nCode				= htonl( nCode );
    //	nMoDataCode			= htonl( nMoDataCode );
    //	nMatchingCode		= htonl( nMatchingCode );
    //	nMoGameCode			= htonl( nMoGameCode );
    //	nOnGameCode			= htonl( nOnGameCode );
    //	nOnGameSerCode		= htonl( nOnGameSerCode );
    //	nOnAbilityCode		= htonl( nOnAbilityCode );
    //	nPowerPoint			= htonl( nPowerPoint );
    //	nContinue			= htonl( nContinue );
    //	nIndex				= htonl( nIndex );

    oStream.write((const char*)this, szPKTPowerPoint);

    // restore order
    //	nSize				= ntohl( nSize );
    //	nCode				= ntohl( nCode );
    //	nMoDataCode			= ntohl( nMoDataCode );
    //	nMatchingCode		= ntohl( nMatchingCode );
    //	nMoGameCode			= ntohl( nMoGameCode );
    //	nOnGameCode			= ntohl( nOnGameCode );
    //	nOnGameSerCode		= ntohl( nOnGameSerCode );
    //	nOnAbilityCode		= ntohl( nOnAbilityCode );
    //	nPowerPoint			= ntohl( nPowerPoint );
    //	nContinue			= ntohl( nContinue );
    //	nIndex				= ntohl( nIndex );
}

// debug message
string PKTPowerPoint::toString() const {
    char str[64];
    StringStream msg;
    msg << "PowerPoint(" << "MoDataCode:" << nMoGameCode;
    memcpy(str, sPhoneNo, 12);
    str[12] = '\0';
    msg << ",PhoneNo:" << str;
    memcpy(str, sMemID, 20);
    str[20] = '\0';
    msg << ",MemID:" << str << ",MatchingCode:" << nMatchingCode;
    memcpy(str, sMoGameName, 20);
    str[20] = '\0';
    msg << ",MoGameName:" << str << ",MoGameCode:" << nMoGameCode << ",OnGameCode:" << nOnGameCode;
    memcpy(str, sOnGameName, 20);
    str[20] = '\0';
    msg << ",OnGameName:" << str << ",OnGameSerCode:" << nOnGameSerCode;
    memcpy(str, sOnGameSerName, 20);
    str[20] = '\0';
    msg << ",OnGameSerName:" << str;
    memcpy(str, sOnGameID, 20);
    str[20] = '\0';
    msg << ",OnGameID:" << str;
    memcpy(str, sCharName, 40);
    str[40] = '\0';
    msg << ",OnCharName:" << str << ",OnAbilityCode:" << nOnAbilityCode;
    memcpy(str, sOnAbilityName, 20);
    str[20] = '\0';
    msg << ",OnAbilityName:" << str << ",PowerPoint:" << nPowerPoint;
    memcpy(str, sInputDate, 20);
    str[20] = '\0';
    msg << ",InputDate:" << str << ",Continue:" << nContinue << ",Index:" << nIndex << ")";

    return msg.toString();
}
