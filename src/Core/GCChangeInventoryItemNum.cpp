//////////////////////////////////////////////////////////////////////
//
// Filename    : GCChangeInventoryItemNum.cpp
// Written By  : elca@ewestsoft.com
// Description : Members of the changed-material record.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCChangeInventoryItemNum.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCChangeInventoryItemNum::GCChangeInventoryItemNum()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCChangeInventoryItemNum::~GCChangeInventoryItemNum()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read the record from the input stream.
//////////////////////////////////////////////////////////////////////
void GCChangeInventoryItemNum::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    BYTE listNum;
    iStream.read(listNum);

    clearChangedItemList();

    int i;
    ObjectID_t item;
    ItemNum_t num;

    for (i = 0; i < listNum; i++) {
        iStream.read(item);
        m_ChangedItemList.push_back(item);
    }
    for (i = 0; i < listNum; i++) {
        iStream.read(num);
        m_ChangedItemNumList.push_back(num);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Write the record to the output stream.
//////////////////////////////////////////////////////////////////////
void GCChangeInventoryItemNum::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_ChangedItemList.size() > kMaxCount)
        throw InvalidProtocolException("too many changed items");

    BYTE listNum = (BYTE)m_ChangedItemList.size();
    oStream.write(listNum);

    for (list<ObjectID_t>::const_iterator itr = m_ChangedItemList.begin(); itr != m_ChangedItemList.end(); itr++) {
        oStream.write(*itr);
    }
    for (list<ItemNum_t>::const_iterator itr2 = m_ChangedItemNumList.begin(); itr2 != m_ChangedItemNumList.end();
         itr2++) {
        oStream.write(*itr2);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// GCChangeInventoryItemNum::addChangedItemListElement()
//
// Put one (item, new count) pair on the list.
//
//////////////////////////////////////////////////////////////////////
void GCChangeInventoryItemNum::addChangedItemListElement(ObjectID_t id, ItemNum_t num)

{
    __BEGIN_TRY

    if (m_ChangedItemList.size() >= kMaxCount)
        throw InvalidProtocolException("too many changed items");

    m_ChangedItemList.push_back(id);
    m_ChangedItemNumList.push_back(num);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCChangeInventoryItemNum::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "Changed ListNum:" << (int)m_ChangedItemList.size() << " ChangedListSet(";

    list<ObjectID_t>::const_iterator itrItem = m_ChangedItemList.begin();
    list<ItemNum_t>::const_iterator itrItemNum = m_ChangedItemNumList.begin();
    for (; itrItem != m_ChangedItemList.end() && itrItemNum != m_ChangedItemNumList.end(); itrItem++, itrItemNum++) {
        msg << "(" << (int)(*itrItem) << "," << (int)(*itrItemNum) << "),";
    }
    msg << ")";
    return msg.toString();

    __END_CATCH
}
