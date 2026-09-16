//////////////////////////////////////////////////////////////////////////////
// Filename    : CGLotterySelect.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGLotterySelect.h"


void CGLotterySelect::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // A byte carries more values than there are lottery types, so it is
    // tested before it is stored.
    BYTE type = 0;
    iStream.read(type);

    if (type >= TYPE_MAX)
        throw InvalidProtocolException("lottery type out of range");

    m_Type = type;

    iStream.read(m_QuestLevel);
    iStream.read(m_GiftID);

    __END_CATCH
}

void CGLotterySelect::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Type);
    oStream.write(m_QuestLevel);
    oStream.write(m_GiftID);

    __END_CATCH
}

string CGLotterySelect::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGLotterySelect(" << "QuestLevel:" << m_QuestLevel << ")";
    return msg.toString();

    __END_CATCH
}
