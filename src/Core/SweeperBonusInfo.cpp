//
// Filename    : SweeperBonusInfo.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "SweeperBonusInfo.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
SweeperBonusInfo::SweeperBonusInfo() {
    __BEGIN_TRY

    m_Race = 0;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
SweeperBonusInfo::~SweeperBonusInfo() {
    __BEGIN_TRY

    clearOptionTypeList();

    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void SweeperBonusInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    //	BYTE szOptionList;

    iStream.read(m_Race);

    /*	iStream.read( szOptionList );
        for ( int i = 0 ; i < szOptionList; i++ )
        {
            OptionType_t optionType;
            iStream.read( optionType );
            m_OptionTypeList.push_back( optionType );
        }
    */
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void SweeperBonusInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write(m_Race);

    /*	BYTE szOptionList = m_OptionTypeList.size();
        oStream.write( szOptionList );

        OptionTypeListConstItor itr = m_OptionTypeList.begin();
        for ( ; itr != m_OptionTypeList.end(); itr++ )
        {
            OptionType_t optionType = *itr;
            oStream.write( optionType );
        }
    */
    __END_CATCH
}

/////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string SweeperBonusInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "SweeperBonusInfo( " << "Race:"
        << (int)m_Race
        //		<< "OptionTypeListSize:" << (int)m_OptionTypeList.size()
        << ")";

    return msg.toString();

    __END_CATCH
}
