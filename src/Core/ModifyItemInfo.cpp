//////////////////////////////////////////////////////////////////////
//
// Filename    : ModifyItemInfo.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "ModifyItemInfo.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
ModifyItemInfo::ModifyItemInfo() {
    __BEGIN_TRY
    m_ListNum = 0;
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
ModifyItemInfo::~ModifyItemInfo() noexcept = default;


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void ModifyItemInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_ListNum);

    DWORD m_Value;
    for (int i = 0; i < m_ListNum; i++) {
        // ObjectID Read
        iStream.read(m_Value);
        m_SList.push_back(m_Value);

        // Modify identifier Read
        iStream.read(m_Value);
        m_SList.push_back(m_Value);

        // Value Read
        iStream.read(m_Value);
        m_SList.push_back(m_Value);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void ModifyItemInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_ListNum);

    for (list<DWORD>::const_iterator itr = m_SList.begin(); itr != m_SList.end(); itr++) {
        oStream.write(*itr);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// ModifyItemInfo::addListElement()
//
// Member function that adds one (changed part, changed value) set to the list.
//
//////////////////////////////////////////////////////////////////////
void ModifyItemInfo::addListElement(ObjectID_t ObjectID, ModifyType List, DWORD Value) {
    __BEGIN_TRY

    m_SList.push_back(ObjectID);

    // Put the wanted modify type into the List.
    m_SList.push_back(List);

    // Put the wanted value into the List.
    m_SList.push_back(Value);

    // Raise the number of changed entries by one.
    m_ListNum++;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string ModifyItemInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "ModifyItemInfo( " << ",ListNum: " << (int)m_ListNum << " ListSet( ";

    for (list<DWORD>::const_iterator itr = m_SList.begin(); itr != m_SList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
