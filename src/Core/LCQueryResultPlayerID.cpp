//--------------------------------------------------------------------------------
//
// Filename    : LCQueryResultPlayerID.cpp
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

// include files
#include "LCQueryResultPlayerID.h"

#include "WireString.h"


//--------------------------------------------------------------------------------
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//--------------------------------------------------------------------------------
void LCQueryResultPlayerID::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    //--------------------------------------------------
    // read player id
    //--------------------------------------------------

    de::wire::readString(iStream, m_PlayerID, {1, 20}, "PlayerID");

    //--------------------------------------------------
    // read id existence
    //--------------------------------------------------
    iStream.read(m_bExist);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//--------------------------------------------------------------------------------
void LCQueryResultPlayerID::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    //--------------------------------------------------
    // write player id
    //--------------------------------------------------
    de::wire::writeString(oStream, m_PlayerID, {1, 20}, "PlayerID");

    //--------------------------------------------------
    // write id existence
    //--------------------------------------------------
    oStream.write(m_bExist);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string LCQueryResultPlayerID::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "LCQueryResultPlayerID(" << "PlayerID:" << m_PlayerID << ",Exist:" << m_bExist << ")";
    return msg.toString();

    __END_CATCH
}
