//--------------------------------------------------------------------------------
//
// Filename    : GCAddMonsterCorpse.cpp
// Written By  : Reiot
//
//--------------------------------------------------------------------------------

// include files
#include "GCAddMonsterCorpse.h"


//--------------------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------------------
void GCAddMonsterCorpse::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    BYTE name_length = 0;

    iStream.read(m_ObjectID);
    iStream.read(m_MonsterType);

    iStream.read(name_length);
    if (name_length > kMaxNameSize)
        throw InvalidProtocolException("too long monster name length");
    if (name_length != 0)
        iStream.read(m_MonsterName, name_length);

    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Dir);
    iStream.read(m_bhasHead);
    iStream.read(m_TreasureCount);
    iStream.read(m_LastKiller);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCAddMonsterCorpse::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_MonsterName.size() > kMaxNameSize)
        throw InvalidProtocolException("too long monster name length");

    BYTE name_length = m_MonsterName.size();

    oStream.write(m_ObjectID);
    oStream.write(m_MonsterType);

    oStream.write(name_length);
    if (m_MonsterName.size() != 0)
        oStream.write(m_MonsterName);

    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Dir);
    oStream.write(m_bhasHead);
    oStream.write(m_TreasureCount);
    oStream.write(m_LastKiller);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCAddMonsterCorpse::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCAddMonsterCorpse(" << "ObjectID:" << m_ObjectID << ",MonsterType:" << (int)m_MonsterType
        << ",MonsterName:" << m_MonsterName << ",X:" << (int)m_X << ",Y:" << (int)m_Y << ",Dir:" << dir2String(m_Dir)
        << ",hasHead:" << (int)m_bhasHead << ",TreasureCount:" << (int)m_TreasureCount << ")";
    return msg.toString();

    __END_CATCH
}
