//////////////////////////////////////////////////////////////////////
//
// Filename    : EffectInfo.cpp
// Written By  : elca@ewestsoft.com
// Description : Effect information: definition of the effect list.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "EffectInfo.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
EffectInfo::EffectInfo() {
    __BEGIN_TRY
    m_ListNum = 0;
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
EffectInfo::~EffectInfo() noexcept = default;


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void EffectInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    // State the actual size when optimizing.
    iStream.read(m_ListNum);

    WORD m_Value;
    for (int i = 0; i < m_ListNum * 2; i++) {
        iStream.read(m_Value);
        m_EList.push_back(m_Value);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void EffectInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // State the actual size when optimizing.
    oStream.write(m_ListNum);

    for (list<WORD>::const_iterator itr = m_EList.begin(); itr != m_EList.end(); itr++) {
        oStream.write(*itr);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// EffectInfo::addListElement()
//
// Member function that appends one ( change type, change value ) pair to the list.
//
//////////////////////////////////////////////////////////////////////
void EffectInfo::addListElement(EffectID_t EffectID, WORD Value) {
    __BEGIN_TRY

    // Put the wanted effect id into the list.
    m_EList.push_back(EffectID);

    // Put the wanted value into the list.
    m_EList.push_back(Value);

    // Increase the number of change entries by one.
    m_ListNum++;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string EffectInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectInfo( " << ",ListNum: " << (int)m_ListNum << " ListSet(";
    for (list<WORD>::const_iterator itr = m_EList.begin(); itr != m_EList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }
    msg << ")";
    return msg.toString();

    __END_CATCH
}
