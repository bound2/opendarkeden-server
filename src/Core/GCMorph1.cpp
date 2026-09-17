//-------------------------------------------------------------------------------- //
// Filename    : GCMorph1.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//--------------------------------------------------------------------------------

// include files
#include "GCMorph1.h"

#include "Assert1.h"
#include "PCSlayerInfo2.h"
#include "PCVampireInfo2.h"

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
GCMorph1::GCMorph1()

    : m_pPCInfo(NULL), m_pInventoryInfo(NULL), m_pGearInfo(NULL), m_pExtraInfo(NULL) {}

//--------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------
GCMorph1::~GCMorph1()

{
    SAFE_DELETE(m_pPCInfo);
    SAFE_DELETE(m_pInventoryInfo);
    SAFE_DELETE(m_pGearInfo);
    SAFE_DELETE(m_pExtraInfo);
}

//--------------------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------------------
void GCMorph1::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    //--------------------------------------------------
    // read pc type/info
    //--------------------------------------------------
    // The four records replace the ones the packet holds.
    SAFE_DELETE(m_pPCInfo);
    SAFE_DELETE(m_pInventoryInfo);
    SAFE_DELETE(m_pGearInfo);
    SAFE_DELETE(m_pExtraInfo);

    char pcType;
    iStream.read(pcType);

    switch (pcType) {
    case 'S':
        m_pPCInfo = new PCSlayerInfo2();
        break;

    case 'V':
        m_pPCInfo = new PCVampireInfo2();
        break;

    default:
        throw InvalidProtocolException("invalid pc type");
    }

    m_pPCInfo->read(iStream);

    m_pInventoryInfo = new InventoryInfo();
    m_pInventoryInfo->read(iStream);

    m_pGearInfo = new GearInfo();
    m_pGearInfo->read(iStream);

    m_pExtraInfo = new ExtraInfo();
    m_pExtraInfo->read(iStream);


    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCMorph1::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    //--------------------------------------------------
    // write pc type
    //--------------------------------------------------
    requireRecords();

    char pcType;
    switch (m_pPCInfo->getPCType()) {
    case PC_SLAYER:
        pcType = 'S';
        break;

    case PC_VAMPIRE:
        pcType = 'V';
        break;

    default:
        throw InvalidProtocolException("invalid pc type");
    }

    oStream.write(pcType);

    m_pPCInfo->write(oStream);

    m_pInventoryInfo->write(oStream);

    m_pGearInfo->write(oStream);

    m_pExtraInfo->write(oStream);


    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCMorph1::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMorph1(" << "PC:" << (m_pPCInfo != NULL ? m_pPCInfo->toString() : string("none")) << ")";
    return msg.toString();

    __END_CATCH
}
