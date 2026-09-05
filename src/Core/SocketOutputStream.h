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
// write a scalar to the output buffer
//
// The ring-buffer walk (and the resize rule that keeps at least one
// byte free, so head == tail can only ever mean "empty") now lives in
// write(std::span<const std::byte>) alone. This template used to carry
// a second, hand-copied version of the same walk -- the copy whose T*
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
