//////////////////////////////////////////////////////////////////////
//
// Filename    : SocketInputStream.h
// Written by  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////
//
// *Reiot's Notes*
//
// �ý��ۿ��� ���� ����ϰ� ���Ǵ� Ŭ�������� �ϳ��̴�.
// �ӵ��� ���������� ������ ��ġ�Ƿ�, ���� ���� �ӵ��� �����ϰ�
// �ʹٸ�, exception�� ���� re-write �϶�.
//
// ���� nonblocking �� ������-�����-���� ���� �߻��Ѵٰ�
// ������, �̰��� NonBlockingIOException���� wrapping�ɶ� overhead�� �߻���
// Ȯ���� ���ٰ� �����ȴ�.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SOCKET_INPUT_STREAM_H__
#define __SOCKET_INPUT_STREAM_H__

// include files
#include <cstddef>
#include <span>

#include "Exception.h"
#include "Socket.h"
#include "Types.h"
#include "WireTypes.h"

// constant definitions
const uint DefaultSocketInputBufferSize = 81920;

// forward declaration
class Packet;

//////////////////////////////////////////////////////////////////////
//
// class SocketInputStream
//
//////////////////////////////////////////////////////////////////////

class SocketInputStream {
    //////////////////////////////////////////////////
    // constructor/destructor
    //////////////////////////////////////////////////
public:
    // constructor
    SocketInputStream(Socket* sock, uint BufferSize = DefaultSocketInputBufferSize);

    // destructor
    virtual ~SocketInputStream() noexcept;


    //////////////////////////////////////////////////
    // methods
    //////////////////////////////////////////////////
public:
    // read data from stream (input buffer)
    //
    // The span overload carries the implementation: it binds the buffer
    // to its length instead of trusting the caller to pass a matching
    // pair. The char*/uint form is kept verbatim for the existing call
    // sites and forwards, so bounds behaviour and the exceptions thrown
    // (InvalidProtocolException on an empty request,
    // InsufficientDataException when the buffer holds less) are the same
    // through either entry point.
    uint read(std::span<std::byte> dst);
    uint read(char* buf, uint len);
    uint read(string& str, uint len);
    void readPacket(Packet* p);

    // Raw scalar read: copies sizeof(T) bytes of the object
    // representation straight off the wire. Only de::WireScalar types
    // may do that -- see WireTypes.h for what that admits and why.
    template <de::WireScalar T> uint read(T& buf);

    // Deliberately an overload rather than only a constraint on the one
    // above: it keeps a rejected type from quietly finding some other
    // viable overload, and reports the type by name instead of "no
    // matching member function".
    template <typename T> uint read(T& buf) {
        static_assert(de::WireScalar<T>,
                      "SocketInputStream::read<T>: T is not a protocol wire scalar. Reading it would copy "
                      "sizeof(T) raw bytes of a pointer, an object layout or a platform-sized integer off the "
                      "wire. Read the protocol's own fixed-width fields (see WireTypes.h), or read a length "
                      "prefix plus read(char*, uint) / read(std::span<std::byte>) for a buffer.");
        return 0;
    }

    /*	uint read (bool   & buf)  { return read((char*)&buf, szbool  ); }
        uint read (char   & buf)  { return read((char*)&buf, szchar  ); }
        uint read (uchar  & buf)  { return read((char*)&buf, szuchar ); }
        uint read (short  & buf)  { return read((char*)&buf, szshort ); }
        uint read (ushort & buf)  { return read((char*)&buf, szushort); }
        uint read (int    & buf)  { return read((char*)&buf, szint   ); }
        uint read (uint   & buf)  { return read((char*)&buf, szuint  ); }
        uint read (long   & buf)  { return read((char*)&buf, szlong  ); }
        uint read (ulong  & buf)  { return read((char*)&buf, szulong ); }
    */
    // peek data from stream (input buffer). Same split as read() above.
    bool peek(std::span<std::byte> dst);
    bool peek(char* buf, uint len);

    // skip data from stream (input buffer)
    void skip(uint len);

    // fill stream (input buffer) from socket
    uint fill();
    uint fill_RAW();

    // resize buffer
    void resize(int size);

    // get buffer length
    uint capacity() const {
        return m_BufferLen;
    }

    // get data length in buffer
    uint length() const;
    uint size() const {
        return length();
    }

    // check if buffer is empty
    bool isEmpty() const {
        return m_Head == m_Tail;
    }

    // get debug string
    string toString() const;


    //////////////////////////////////////////////////
    // attributes
    //////////////////////////////////////////////////
private:
    // socket
    Socket* m_pSocket;

    // buffer
    char* m_Buffer;

    // buffer length
    uint m_BufferLen;

    // buffer head/tail
    uint m_Head;
    uint m_Tail;
    // add by viva 2008-12-31
public:
    WORD m_EncryptKey;
    BYTE* m_HashTable;
    void setKey(WORD EncryptKey, BYTE* HashTable) {
        m_EncryptKey = EncryptKey;
        m_HashTable = HashTable;
    }
    WORD EncryptData(WORD EncryptKey, char* buf, int len);
    // end
};


//////////////////////////////////////////////////////////////////////
//
// read a scalar from the input buffer
//
// The ring-buffer walk now lives in read(std::span<std::byte>) alone.
// This template used to carry a second, hand-copied version of the
// same walk -- the copy whose T* cast at an arbitrary buffer offset
// tripped UBSan and was changed to memcpy (docs/TOOLCHAIN.md section
// 2). Behaviour is unchanged: sizeof(T) is never 0 and &buf is never
// null, so neither extra guard in the span overload can fire here.
//
// reinterpret_cast to std::byte* is not an aliasing violation: std::byte
// (like char) may alias any object. std::bit_cast would need a second
// copy through a temporary array to say the same thing, so it is not
// used here.
//
//////////////////////////////////////////////////////////////////////
template <de::WireScalar T> uint SocketInputStream::read(T& buf) {
    return read(std::span<std::byte>(reinterpret_cast<std::byte*>(&buf), sizeof(T)));
}

#endif
