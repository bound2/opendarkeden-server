//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMiniGameScores.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCMiniGameScores.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCMiniGameScores::GCMiniGameScores()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCMiniGameScores::~GCMiniGameScores()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCMiniGameScores::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_GameType);
    iStream.read(m_Level);

    // The table replaces the one the packet holds.
    m_Scores.clear();

    BYTE count;
    iStream.read(count);

    if (count > kMaxScores)
        throw InvalidProtocolException("too many mini game scores");

    for (BYTE i = 0; i < count; ++i) {
        string name;
        de::wire::readString(iStream, name, {0, kMaxNameLength}, "MiniGameName");
        WORD score;
        iStream.read(score);

        addScore(name, score);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCMiniGameScores::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write(m_GameType);
    oStream.write(m_Level);

    const uint count = m_Scores.size() > kMaxScores ? kMaxScores : (uint)m_Scores.size();
    oStream.write((BYTE)count);

    list<pair<string, WORD>>::const_iterator itr = m_Scores.begin();

    for (uint i = 0; i < count; ++i) {
        de::wire::writeString(oStream, (*itr).first, {0, kMaxNameLength}, "MiniGameName");
        oStream.write((*itr).second);
        itr++;
    }

    __END_CATCH
}

PacketSize_t GCMiniGameScores::getPacketSize() const {
    PacketSize_t ret = szBYTE + szBYTE + szBYTE;

    const uint count = m_Scores.size() > kMaxScores ? kMaxScores : (uint)m_Scores.size();

    list<pair<string, WORD>>::const_iterator itr = m_Scores.begin();

    for (uint i = 0; i < count; ++i) {
        ret += de::wire::stringWireSize((*itr).first) + szWORD;
        itr++;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCMiniGameScores::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMiniGameScores(" << ")";
    return msg.toString();

    __END_CATCH
}
