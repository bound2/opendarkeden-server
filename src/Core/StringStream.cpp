//////////////////////////////////////////////////////////////////////
//
// Filename    : StringStream.cc
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "StringStream.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
StringStream::StringStream() : m_Size(0), m_bInserted(false), m_Buffer("") {}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
StringStream::~StringStream() {}


//////////////////////////////////////////////////////////////////////
// add string to stream
//////////////////////////////////////////////////////////////////////
StringStream& StringStream::operator<<(bool T) {
    string buf(T == true ? "true" : "false");

    m_Strings.push_back(buf);

    m_Size += buf.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(char T) {
    string buf(1, '\0');
    buf[0] = T;

    m_Strings.push_back(buf);

    m_Size += buf.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(uchar T) {
    string buf(1, 0);
    buf[0] = T;

    m_Strings.push_back(buf);

    m_Size += buf.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(short T) {
    char buf[7];
    sprintf(buf, "%d", T);

    string str(buf);

    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(ushort T) {
    char buf[7];
    sprintf(buf, "%u", T);

    string str(buf);

    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(int T) {
    char buf[12];
    sprintf(buf, "%d", T);

    string str(buf);

    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(uint T) {
    char buf[12];
    sprintf(buf, "%u", T);

    string str(buf);

    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(long T) {
    // 64-bit on the LP64 build target: LONG_MIN prints as 20 digits plus a
    // sign, so the 12-byte buffer this was copy-pasted from the int overload
    // with overflowed the stack for any value outside the 32-bit range.
    char buf[24];
    snprintf(buf, sizeof(buf), "%ld", T);

    string str(buf);

    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(ulong T) {
    // See operator<<(long): 64-bit here, ULONG_MAX is 20 digits.
    char buf[24];
    snprintf(buf, sizeof(buf), "%lu", T);

    string str(buf);

    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(float T) {
    // Same defect as operator<<(long) had: "%f" prints the number in full,
    // never in exponent form, plus six decimals. A float argument is promoted
    // to double here, so FLT_MAX needs 39 integer digits + '.' + 6 + sign.
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", T);

    string str(buf);

    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(double T) {
    // See operator<<(float): DBL_MAX is 309 integer digits under "%f".
    char buf[352];
    snprintf(buf, sizeof(buf), "%f", T);

    string str(buf);

    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(const char* buf) {
    string str(buf);

    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}

StringStream& StringStream::operator<<(const string& str) {
    m_Strings.push_back(str);

    m_Size += str.size();
    m_bInserted = true;

    return *this;
}


//////////////////////////////////////////////////////////////////////
// make string
//////////////////////////////////////////////////////////////////////
const string& StringStream::toString() const {
    // 일단 스트링을 한번 생성해놓으면,
    // 그다음 호출때에는 새로 추가되지 않는 한 그대로 사용한다.
    if (m_bInserted) {
        m_bInserted = false;

        // 속도를 위해 쓸데없는 복사 방지를 일단 메모리를 다 잡아놓고 시작한다.
        m_Buffer.reserve(m_Size);

        for (list<string>::const_iterator itr = m_Strings.begin(); itr != m_Strings.end(); itr++) {
            // 버퍼에 하나씩 추가한다.
            m_Buffer.append(*itr);
        }
    }

    return m_Buffer;
}
