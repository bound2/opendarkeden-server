//-------------------------------------------------------------------------------- //
// Filename    : GCRankBonusInfo.cpp
// Written By  : elca@ewestsoft.com
// Description :
//
//--------------------------------------------------------------------------------

// include files
#include "GCRankBonusInfo.h"

#include "Assert1.h"

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
GCRankBonusInfo::GCRankBonusInfo()

{}

//--------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------
GCRankBonusInfo::~GCRankBonusInfo()

{
    m_RankBonusInfoList.clear();
}

//--------------------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------------------
void GCRankBonusInfo::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The listing replaces the one the packet holds.
    m_RankBonusInfoList.clear();

    BYTE ListNum;
    iStream.read(ListNum);

    if (ListNum > kMaxEntries)
        throw InvalidProtocolException("too many rank bonuses");

    for (WORD i = 0; i < ListNum; i++) {
        DWORD rankBonusType;
        iStream.read(rankBonusType);
        m_RankBonusInfoList.push_back(rankBonusType);
    }

    __END_CATCH
}

//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCRankBonusInfo::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_RankBonusInfoList.size() > kMaxEntries)
        throw InvalidProtocolException("too many rank bonuses");

    BYTE size = m_RankBonusInfoList.size();
    oStream.write(size);

    for (list<DWORD>::const_iterator itr = m_RankBonusInfoList.begin(); itr != m_RankBonusInfoList.end(); itr++) {
        oStream.write((*itr));
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCRankBonusInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCRankBonusInfo(";
    for (list<DWORD>::const_iterator itr = m_RankBonusInfoList.begin(); itr != m_RankBonusInfoList.end(); itr++) {
        msg << (*itr) << ",";
    }
    msg << ")";

    return msg.toString();

    __END_CATCH
}
