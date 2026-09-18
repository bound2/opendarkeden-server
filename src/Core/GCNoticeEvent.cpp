//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNoticeEvent.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCNoticeEvent.h"

//////////////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////////////
void GCNoticeEvent::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    WORD code = 0;
    iStream.read(code);

    if (code >= NOTICE_EVENT_MAX)
        throw InvalidProtocolException("unknown notice event code");

    m_Code = code;

    switch (m_Code) {
    // Codes that need a parameter
    case NOTICE_EVENT_MASTER_COMBAT_TIME:
    case NOTICE_EVENT_KICK_OUT_FROM_ZONE:
    case NOTICE_EVENT_CONTINUAL_GROUND_ATTACK:
    case NOTICE_EVENT_METEOR_STRIKE:
    case NOTICE_EVENT_SHOP_TAX_CHANGE:
    case NOTICE_EVENT_WAR_OVER:
    case NOTICE_EVENT_RESULT_LOTTERY:
    case NOTICE_EVENT_MASTER_LAIR_OPEN:
    case NOTICE_EVENT_MASTER_LAIR_CLOSED:
    case NOTICE_EVENT_MASTER_LAIR_COUNT:
    case NOTICE_EVENT_MINI_GAME:
    case NOTICE_EVENT_FLAG_WAR_READY:
    case NOTICE_EVENT_LEVEL_WAR_ARRANGED:
    case NOTICE_EVENT_LEVEL_WAR_STARTED:
    case NOTICE_EVENT_RACE_WAR_SOON:
    case NOTICE_EVENT_ENTER_BEGINNER_ZONE:
    case NOTICE_EVENT_LOGIN_JUST_NOW:
    case NOTICE_EVENT_HOLYDAY:
    case NOTICE_EVENT_GOLD_MEDALS:
    case NOTICE_EVENT_CROWN_PRICE:
        iStream.read(m_Parameter);
        break;
    // Codes that need no parameter
    default:
        break;
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////////////
void GCNoticeEvent::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Code);

    switch (m_Code) {
    // Codes that need a parameter
    case NOTICE_EVENT_MASTER_COMBAT_TIME:
    case NOTICE_EVENT_KICK_OUT_FROM_ZONE:
    case NOTICE_EVENT_CONTINUAL_GROUND_ATTACK:
    case NOTICE_EVENT_METEOR_STRIKE:
    case NOTICE_EVENT_SHOP_TAX_CHANGE:
    case NOTICE_EVENT_WAR_OVER:
    case NOTICE_EVENT_RESULT_LOTTERY:
    case NOTICE_EVENT_MASTER_LAIR_OPEN:
    case NOTICE_EVENT_MASTER_LAIR_CLOSED:
    case NOTICE_EVENT_MASTER_LAIR_COUNT:
    case NOTICE_EVENT_MINI_GAME:
    case NOTICE_EVENT_FLAG_WAR_READY:
    case NOTICE_EVENT_LEVEL_WAR_ARRANGED:
    case NOTICE_EVENT_LEVEL_WAR_STARTED:
    case NOTICE_EVENT_RACE_WAR_SOON:
    case NOTICE_EVENT_ENTER_BEGINNER_ZONE:
    case NOTICE_EVENT_LOGIN_JUST_NOW:
    case NOTICE_EVENT_HOLYDAY:
    case NOTICE_EVENT_GOLD_MEDALS:
    case NOTICE_EVENT_CROWN_PRICE:
        oStream.write(m_Parameter);
        break;
    // Codes that need no parameter
    default:
        break;
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Packet size
//////////////////////////////////////////////////////////////////////////////

PacketSize_t GCNoticeEvent::getPacketSize() const

{
    __BEGIN_TRY

    PacketSize_t size = szWORD;

    switch (m_Code) {
    // Codes that need a parameter
    case NOTICE_EVENT_MASTER_COMBAT_TIME:
    case NOTICE_EVENT_KICK_OUT_FROM_ZONE:
    case NOTICE_EVENT_CONTINUAL_GROUND_ATTACK:
    case NOTICE_EVENT_METEOR_STRIKE:
    case NOTICE_EVENT_SHOP_TAX_CHANGE:
    case NOTICE_EVENT_WAR_OVER:
    case NOTICE_EVENT_RESULT_LOTTERY:
    case NOTICE_EVENT_MASTER_LAIR_OPEN:
    case NOTICE_EVENT_MASTER_LAIR_CLOSED:
    case NOTICE_EVENT_MASTER_LAIR_COUNT:
    case NOTICE_EVENT_MINI_GAME:
    case NOTICE_EVENT_FLAG_WAR_READY:
    case NOTICE_EVENT_LEVEL_WAR_ARRANGED:
    case NOTICE_EVENT_LEVEL_WAR_STARTED:
    case NOTICE_EVENT_RACE_WAR_SOON:
    case NOTICE_EVENT_ENTER_BEGINNER_ZONE:
    case NOTICE_EVENT_LOGIN_JUST_NOW:
    case NOTICE_EVENT_HOLYDAY:
    case NOTICE_EVENT_GOLD_MEDALS:
    case NOTICE_EVENT_CROWN_PRICE:
        size += szuint;
        break;
    // Codes that need no parameter
    default:
        break;
    }

    return size;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCNoticeEvent::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCNoticeEvent(" << "Code : " << (int)m_Code << "Parameter : " << (int)m_Parameter << ")";
    return msg.toString();

    __END_CATCH
}
