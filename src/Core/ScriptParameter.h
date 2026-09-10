//////////////////////////////////////////////////////////////////////
//
// Filename    : ScriptParameter.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __SCRIPT_PARAMETER_H__
#define __SCRIPT_PARAMETER_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "Types.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class ScriptParameter;
//
//
//
//////////////////////////////////////////////////////////////////////

class ScriptParameter {
public:
    // constructor
    ScriptParameter();

    // destructor
    ~ScriptParameter() noexcept;

public:
    // �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ��
    // �ʱ�ȭ�Ѵ�.
    void read(SocketInputStream& iStream);

    // ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // ����ȭ��, �̸� ���� ������ ����Ѵ�.
    PacketSize_t getSize();

    // Each string travels behind a BYTE length.
    static constexpr uint kMaxStringSize = de::wire::kMaxByteStringLength;

    static constexpr uint getMaxSize() {
        return (szBYTE + kMaxStringSize) * 2;
    }

    // get packet's debug string
    string toString() const;

    // get/set Name
    string getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    // get/set Value
    string getValue() const {
        return m_Value;
    }
    void setValue(const string& value) {
        m_Value = value;
    }

private:
    string m_Name;
    string m_Value;
};

#endif
