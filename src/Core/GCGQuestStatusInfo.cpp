//////////////////////////////////////////////////////////////////////
//
// Filename    : GCGQuestStatusInfo.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCGQuestStatusInfo.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCGQuestStatusInfo::GCGQuestStatusInfo()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCGQuestStatusInfo::~GCGQuestStatusInfo()

{
    __BEGIN_TRY

    clearInfos();

    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Drop the listing, freeing only the records read() allocated.
//////////////////////////////////////////////////////////////////////
void GCGQuestStatusInfo::clearInfos()

{
    if (m_bOwnsInfos) {
        list<QuestStatusInfo*>::iterator itr = m_Infos.begin();
        for (; itr != m_Infos.end(); ++itr)
            delete *itr;
    }

    m_Infos.clear();
    m_bOwnsInfos = false;
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCGQuestStatusInfo::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The listing replaces the one the packet holds.
    clearInfos();

    BYTE size;
    iStream.read(size);

    if (size > MAX_QUEST_NUM)
        throw InvalidProtocolException("too many quest status records");

    m_bOwnsInfos = true;

    for (int i = 0; i < size; ++i) {
        QuestStatusInfo* pInfo = new QuestStatusInfo(0);
        m_Infos.push_back(pInfo);
        pInfo->read(iStream);
    }


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCGQuestStatusInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_Infos.size() > MAX_QUEST_NUM)
        throw InvalidProtocolException("too many quest status records");

    BYTE size = m_Infos.size();
    oStream.write(size);

    list<QuestStatusInfo*>::const_iterator itr = m_Infos.begin();

    for (; itr != m_Infos.end(); ++itr) {
        (*itr)->write(oStream);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCGQuestStatusInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCGQuestStatusInfo(" << ")";
    return msg.toString();

    __END_CATCH
}
