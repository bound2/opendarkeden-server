
//////////////////////////////////////////////////////////////////////////////
// Filename    : CGMixItem.cpp
// Written By  : excel96
// Description :
// When an item in the inventory is used, the client sends X, Y and the ObjectID;
// the server then runs the code that matches the item's class.
//////////////////////////////////////////////////////////////////////////////

#include "CGMixItem.h"

#include "Assert1.h"
#include "SocketEncryptInputStream.h"
#include "SocketEncryptOutputStream.h"


void CGMixItem::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_InvenX);
    iStream.read(m_InvenY);
    iStream.read(m_TargetObjectID[0]);
    iStream.read(m_TargetObjectID[1]);

    __END_CATCH
}

void CGMixItem::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_InvenX);
    oStream.write(m_InvenY);
    oStream.write(m_TargetObjectID[0]);
    oStream.write(m_TargetObjectID[1]);

    __END_CATCH
}

string CGMixItem::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGMixItem(" << "ObjectID:" << (int)m_ObjectID << ",InvenX:" << (int)m_InvenX << ",InvenY:" << (int)m_InvenY
        << ",ObjectID:" << (int)m_TargetObjectID[0] << ",ObjectID:" << (int)m_TargetObjectID[1] << ")";
    return msg.toString();

    __END_CATCH
}
