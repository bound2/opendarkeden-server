//////////////////////////////////////////////////////////////////////
//
// Filename    : BloodBibleSignInfo.cpp
// Written By  : elca@ewestsoft.com
// Description : Member definitions of the packet class that reports the
//               success of a skill used on oneself.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "BloodBibleSignInfo.h"

#include "Assert.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
BloodBibleSignInfo::BloodBibleSignInfo() {
    __BEGIN_TRY

    m_OpenNum = 0;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
BloodBibleSignInfo::~BloodBibleSignInfo() {
    __BEGIN_TRY

    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void BloodBibleSignInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    // The signs replace the ones the record holds.
    m_SignList.clear();

    iStream.read(m_OpenNum);
    BYTE num;
    iStream.read(num);

    if (num > BLOOD_BIBLE_SIGN_SLOT_NUM)
        throw InvalidProtocolException("too many blood bible signs");

    for (int i = 0; i < num; ++i) {
        ItemType_t type;
        iStream.read(type);
        m_SignList.push_back(type);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void BloodBibleSignInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write(m_OpenNum);
    BYTE num = signCount();
    oStream.write(num);
    for (int i = 0; i < num; ++i) {
        oStream.write(m_SignList[i]);
    }

    __END_CATCH
}
