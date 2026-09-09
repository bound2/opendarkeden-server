//////////////////////////////////////////////////////////////////////////////
// Filename    : CGRequestPowerPoint.cpp
// Written By  :
// Description :
// Request the target player's IP address.
//////////////////////////////////////////////////////////////////////////////

#include "CGRequestPowerPoint.h"

#include "WireString.h"

//////////////////////////////////////////////////////////////////////////////
// class CGRequestPowerPoint member methods
//////////////////////////////////////////////////////////////////////////////

CGRequestPowerPoint::CGRequestPowerPoint()

    {__BEGIN_TRY __END_CATCH}

CGRequestPowerPoint::~CGRequestPowerPoint()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void CGRequestPowerPoint::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_CellNum, {1, 12}, "CellNum");

    __END_CATCH
}

void CGRequestPowerPoint::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    de::wire::writeString(oStream, m_CellNum, {1, 12}, "CellNum");

    __END_CATCH
}

string CGRequestPowerPoint::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGRequestPowerPoint(" << ",CellNum: " << m_CellNum << ")";
    return msg.toString();

    __END_CATCH
}
