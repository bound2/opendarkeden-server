//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMoveOK.cc
// Written By  : elca@ewestsoft.com
// Description : Definition of the packet class functions that send an OK
//               sign back to the sender when the request arrives
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCMoveOK.h"

#include "Assert1.h"
#include "SocketEncryptInputStream.h"
#include "SocketEncryptOutputStream.h"

//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCMoveOK::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    /*
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Dir);
    */

#ifdef __USE_ENCRYPTER__
    SocketEncryptInputStream* pEIStream = dynamic_cast<SocketEncryptInputStream*>(&iStream);
    Assert(pEIStream != NULL);

    if (pEIStream->getEncryptCode() != 0) {
        SHUFFLE_STATEMENT_3(pEIStream->getEncryptCode(), pEIStream->readEncrypt(m_X), pEIStream->readEncrypt(m_Y),
                            pEIStream->readEncrypt(m_Dir));
    } else
#endif
    {
        iStream.read(m_X);
        iStream.read(m_Y);
        iStream.read(m_Dir);
    }


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCMoveOK::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    /*
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Dir);
    */

#ifdef __USE_ENCRYPTER__
    SocketEncryptOutputStream* pEOStream = dynamic_cast<SocketEncryptOutputStream*>(&oStream);
    Assert(pEOStream != NULL);

    if (pEOStream->getEncryptCode() != 0) {
        SHUFFLE_STATEMENT_3(pEOStream->getEncryptCode(), pEOStream->writeEncrypt(m_X), pEOStream->writeEncrypt(m_Y),
                            pEOStream->writeEncrypt(m_Dir));
    } else
#endif
    {
        oStream.write(m_X);
        oStream.write(m_Y);
        oStream.write(m_Dir);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCMoveOK::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMoveOK(" << "X:" << (int)m_X << ",Y:" << (int)m_Y << ",Dir:" << dir2String(m_Dir) << ")";
    return msg.toString();

    __END_CATCH
}
