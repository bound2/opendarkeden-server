//////////////////////////////////////////////////////////////////////
//
// SocketOutputStream.h
//
// by Reiot
//
//////////////////////////////////////////////////////////////////////

#ifndef __SOCKET_OUTPUT_STREAM_H__
#define __SOCKET_OUTPUT_STREAM_H__

// include files
#include <cstddef>
#include <cstring>
#include <span>

#include "Exception.h"
#include "Socket.h"
#include "Types.h"
#include "WireTypes.h"

// constant definitions
const unsigned int DefaultSocketOutputBufferSize = 81920;

// forward declaration
class Packet;

//////////////////////////////////////////////////////////////////////
//
// class SocketOutputStream
//
//////////////////////////////////////////////////////////////////////

class SocketOutputStream {
    //////////////////////////////////////////////////
    // constructor/destructor
    //////////////////////////////////////////////////
public:
    // constructor
    SocketOutputStream(Socket* sock, uint BufferSize = DefaultSocketOutputBufferSize);

    // destructor
    virtual ~SocketOutputStream() noexcept;


    //////////////////////////////////////////////////
    // methods
    //////////////////////////////////////////////////
public:
    // write data to stream (output buffer)
    //
    // *CAUTION* (translated from the original Korean note)
    // Writing a string to the buffer could prefix its size
    // automatically, but whether that size should be a BYTE or a WORD
    // is an open question. Packets are kept as small as possible, so
    // the caller writes the string size as a BYTE or a WORD by hand,
    // whichever the packet needs.
    //
    // The span overload carries the implementation: it binds the buffer
    // to its length instead of trusting the caller to pass a matching
    // pair. The const char*/uint form is kept verbatim for the existing
    // call sites and forwards to it, so the resize behaviour and the
    // bytes produced are the same through either entry point.
    uint write(std::span<const std::byte> src);
    uint write(const char* buf, uint len);
    uint write(const string& buf) {
        return write(buf.c_str(), buf.size());
    }
    void writePacket(const Packet* pPacket);

    // Raw scalar write: copies sizeof(T) bytes of the object
    // representation straight onto the wire. Only de::WireScalar types
    // may do that -- see WireTypes.h for what that admits and why.
    template <de::WireScalar T> uint write(T buf);

    // Deliberately an overload rather than only a constraint on the one
    // above. Without it, constraining write<T> away would let a
    // `write(someCharPointer)` fall back to write(const string&) through
    // a user-defined conversion and silently put a *different* byte
    // sequence on the wire. This keeps it a compile error, named.
    template <typename T> uint write(T buf) {
        static_assert(de::WireScalar<T>,
                      "SocketOutputStream::write<T>: T is not a protocol wire scalar. Writing it would put "
                      "sizeof(T) raw bytes of a pointer, an object layout or a platform-sized integer on the "
                      "wire. Write the protocol's own fixed-width fields (see WireTypes.h), or write a length "
                      "prefix plus write(const char*, uint) / write(std::span<const std::byte>) for a buffer.");
        return 0;
    }
    /*	uint write (bool   buf)  { return write((const char*)&buf, szbool  ); }
        uint write (char   buf)  { return write((const char*)&buf, szchar  ); }
        uint write (uchar  buf)  { return write((const char*)&buf, szuchar ); }
        uint write (short  buf)  { return write((const char*)&buf, szshort ); }
        uint write (ushort buf)  { return write((const char*)&buf, szushort); }
        uint write (int    buf)  { return write((const char*)&buf, szint   ); }
        uint write (uint   buf)  { return write((const char*)&buf, szuint  ); }
        uint write (long   buf)  { return write((const char*)&buf, szlong  ); }
        uint write (ulong  buf)  { return write((const char*)&buf, szulong ); }
    */
    // flush stream (output buffer) to socket
    uint flush();

    // resize buffer
    void resize(int size);

    // get buffer length
    int capacity() const {
        return m_BufferLen;
    }

    // get data length in buffer
    uint length() const;
    uint size() const {
        return length();
    }

    // get data in buffer
    char* getBuffer() const {
        return m_Buffer;
    }

    // check if buffer is empty
    bool isEmpty() const {
        return m_Head == m_Tail;
    }

    // get debug string
    string toString() const {
        StringStream msg;
        msg << "SocketOutputStream(m_BufferLen:" << m_BufferLen << ",m_Head:" << m_Head << ",m_Tail:" << m_Tail << ")";
        return msg.toString();
    }

    //////////////////////////////////////////////////
    // attributes
    //////////////////////////////////////////////////
private:
    // socket
    Socket* m_Socket;

    // output buffer
    char* m_Buffer;

    // buffer length
    uint m_BufferLen;

    // buffer head/tail
    uint m_Head;
    uint m_Tail;

    // �������
    BYTE m_Sequence;
    // add by viva 2008-12-31
public:
    WORD m_EncryptKey;
    BYTE* m_HashTable;
    void setKey(WORD EncryptKey, BYTE* HashTable) {
        m_EncryptKey = EncryptKey;
        m_HashTable = HashTable;
    };
    WORD EncryptData(WORD EncryptKey, char* buf, int len);
    // end
};


//////////////////////////////////////////////////////////////////////
//
// write data to stream (output buffer)
//
// The one copy of the ring-buffer walk: the const char*/uint overload
// and the scalar write<T>() template both come through here. The span
// binds the source to its own length, so the two can no longer
// disagree.
//
// Defined inline, in the header, ON PURPOSE. write<T>() calls this with
// src.size() == sizeof(T); out of line that becomes a call with a runtime
// length, and the memcpy below stops folding to the single store that
// docs/TOOLCHAIN.md section 2 relies on. Every scalar field of every
// packet goes through here, so keep the definition visible.
//
// *Notes*
//
// ( ( m_Head = m_Tail + 1 ) ||
//   ( ( m_Head == 0 ) && ( m_Tail == m_BufferLen - 1 ) )
//
// is what "full" means, so one byte of the buffer is always left
// unused: that is how m_Head == m_Tail can mean "empty" and never
// "full". (The original Korean note did not survive the encoding
// migration.)
//
//////////////////////////////////////////////////////////////////////
inline uint SocketOutputStream::write(std::span<const std::byte> src) {
    __BEGIN_TRY

    const char* buf = reinterpret_cast<const char*>(src.data());
    const uint len = (uint)src.size();

    // Free space left in the buffer. The m_Head > m_Tail case was
    // revised repeatedly -- by sigi. 2002.9.16 / 2002.9.23 / 2002.9.27 --
    // and the dead expression below is what it was revised away from.
    // (The original Korean note did not survive the encoding migration.)
    uint nFree = ((m_Head <= m_Tail) ? m_BufferLen - m_Tail + m_Head - 1 : m_Head - m_Tail - 1);
    // m_Tail - m_Head - 1 );

    // Grow the buffer when the data does not fit in the free space.
    if (len >= nFree)
        resize(len - nFree + 1);

    if (m_Head <= m_Tail) { // normal order

        //
        //    H   T
        // 0123456789
        // ...abcd...
        //

        if (m_Head == 0) {
            nFree = m_BufferLen - m_Tail - 1;
            memcpy(&m_Buffer[m_Tail], buf, len);

        } else {
            nFree = m_BufferLen - m_Tail;
            if (len <= nFree)
                memcpy(&m_Buffer[m_Tail], buf, len);
            else {
                memcpy(&m_Buffer[m_Tail], buf, nFree);
                memcpy(m_Buffer, &buf[nFree], len - nFree);
            }
        }

    } else { // reversed order

        //
        //     T  H
        // 0123456789
        // abcd...efg
        //

        memcpy(&m_Buffer[m_Tail], buf, len);
    }

    // advance m_Tail
    m_Tail = (m_Tail + len) % m_BufferLen;

    return len;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// write a scalar to the output buffer
//
// The ring-buffer walk (and the resize rule that keeps at least one
// byte free, so head == tail can only ever mean "empty") now lives in
// the write(std::span<const std::byte>) definition just above --
// inline, so sizeof(T) still reaches its memcpy as a constant. This
// template used to carry a second, hand-copied version of the same
// walk -- the copy whose T*
// cast at an arbitrary buffer offset tripped UBSan and was changed to
// memcpy (docs/TOOLCHAIN.md section 2). Behaviour is unchanged.
//
// reinterpret_cast to std::byte* is not an aliasing violation:
// std::byte (like char) may alias any object. std::bit_cast would need
// a second copy through a temporary array to say the same thing, so it
// is not used here.
//
//////////////////////////////////////////////////////////////////////
template <de::WireScalar T> uint SocketOutputStream::write(T buf) {
    return write(std::span<const std::byte>(reinterpret_cast<const std::byte*>(&buf), sizeof(T)));
}

#endif
