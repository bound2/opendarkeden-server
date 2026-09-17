//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddItemToItemVerify.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_ITEM_TO_ITEM_VERIFY_H__
#define __GC_ADD_ITEM_TO_ITEM_VERIFY_H__

#include "Packet.h"
#include "PacketFactory.h"

enum {
    ADD_ITEM_TO_ITEM_VERIFY_ERROR, // error

    ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_IMPOSSIBLE,            // enchant completely impossible
    ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL_CRASH,            // enchant failed: the item broke
    ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL_DECREASE,         // enchant failed: an option was lost
    ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK,                    // enchant succeeded
    ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_IMPOSSIBLE_NO_PREMIUM, // enchant impossible: not premium

    ADD_ITEM_TO_ITEM_VERIFY_TRANS_OK,
    ADD_ITEM_TO_ITEM_VERIFY_TRANS_IMPOSSIBLE,

    ADD_ITEM_TO_ITEM_VERIFY_MIXING_OK,
    ADD_ITEM_TO_ITEM_VERIFY_DETACHING_OK,

    ADD_ITEM_TO_ITEM_VERIFY_MIXING_FAILED_SAME_OPTION_GROUP, // cannot mix: the option groups are the same.
    ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL,                    // enchant failed

    ADD_ITEM_TO_ITEM_VERIFY_REVIVAL_OK, // pet revival succeeded

    ADD_ITEM_TO_ITEM_VERIFY_CLEAR_OPTION_OK, // option removal succeeded
    ADD_ITEM_TO_ITEM_VERIFY_UP_GRADE_OK,     // grade raise succeeded

    // add by Coffee 2006.11.3
    ADD_ITEM_TO_ITEM_VERIFY_THREE_ENCHANT_OK,
    //
    ADD_ITEM_TO_ITEM_VERIFY_MAX
};

//////////////////////////////////////////////////////////////////////////////
// class GCAddItemToItemVerify
//////////////////////////////////////////////////////////////////////////////

class GCAddItemToItemVerify : public Packet {
public:
    GCAddItemToItemVerify() {
        m_Code = ADD_ITEM_TO_ITEM_VERIFY_MAX;
        m_Parameter = 0;
        m_Parameter2 = 0;
    }
    virtual ~GCAddItemToItemVerify() {}

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_ITEM_TO_ITEM_VERIFY;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCAddItemToItemVerify";
    }
    string toString() const;

public:
    BYTE getCode(void) const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

    uint getParameter(void) const {
        return m_Parameter;
    }

    void setParameter(uint parameter) {
        m_Parameter = parameter;
    }
    // add by Coffee 2006.11.3
    uint getParameter2(void) const {
        return m_Parameter2;
    }
    void setParameter2(uint parameter2) {
        m_Parameter2 = parameter2;
    }

private:
    BYTE m_Code;
    uint m_Parameter;
    uint m_Parameter2;
};


//////////////////////////////////////////////////////////////////////////////
// class GCAddItemToItemVerifyFactory;
//////////////////////////////////////////////////////////////////////////////

class GCAddItemToItemVerifyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_ITEM_TO_ITEM_VERIFY;
    static constexpr std::string_view kName = "GCAddItemToItemVerify";
    static constexpr PacketSize_t kMaxSize{szBYTE + szuint + szuint};

    Packet* createPacket() override {
        return new GCAddItemToItemVerify();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


#endif
