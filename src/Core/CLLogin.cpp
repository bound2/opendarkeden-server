//////////////////////////////////////////////////////////////////////////////
// Filename    : CLLogin.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLLogin.h"

#include "Properties.h"
#include "WireString.h"

void CLLogin::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    setNetmarble(false);

    de::wire::readString(iStream, m_ID, {1, 30}, "ID");

    de::wire::readString(iStream, m_Password, {1, 30}, "Password");

    iStream.read((char*)m_cMacAddress, 6 * szBYTE);

    iStream.read(m_LoginMode);

    /* convert hex -> str */
    // char tmpStr[20];
    // sprintf(tmpStr, "%02x%02x%02x%02x%02x%02x",
    // m_cMacAddress[0],m_cMacAddress[1],m_cMacAddress[2],m_cMacAddress[3],m_cMacAddress[4],m_cMacAddress[5]);
    // m_strMacAddress = tmpStr[i];

    __END_CATCH
}

void CLLogin::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // 넷마블의 Cpsso 관련된 코드는 서버의 Write 에서는 고치지 않는다  (쓰이지 않으므로 ;;)
    // Client 에서만 알아서 처리해서 보내주도록 한다.
    de::wire::writeString(oStream, m_ID, {1, 30}, "ID");

    de::wire::writeString(oStream, m_Password, {1, 30}, "Password");
    oStream.write((char*)m_cMacAddress, 6 * sizeof(BYTE));

    oStream.write(m_LoginMode);

    __END_CATCH
}

string CLLogin::toString() const

{
    StringStream msg;
    msg << "CLLogin(" << "ID:" << m_ID << ",Password:<redacted>" << ")";
    return msg.toString();
}

PacketSize_t CLLogin::getPacketSize() const

{
    return szBYTE + m_ID.size() + szBYTE + m_Password.size() + 6 + szBYTE;
}

bool CLLogin::checkMacAddress(string lastMac) const {
    bool retValue = false;

    char tmpStr[13];
    sprintf(tmpStr, "%02x%02x%02x%02x%02x%02x", m_cMacAddress[0], m_cMacAddress[1], m_cMacAddress[2], m_cMacAddress[3],
            m_cMacAddress[4], m_cMacAddress[5]);
    tmpStr[12] = '\0';

    if (tmpStr == lastMac)
        retValue = true;
    // if(m_strMacAddress	== lastMac)	retValue = true;

    return retValue;
}
