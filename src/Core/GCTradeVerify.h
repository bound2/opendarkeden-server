////////////////////////////////////////////////////////////////////////////////
// Filename    : GCTradeVerify.h
// Written By  : 김성민
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __GC_TRADE_VERIFY_H__
#define __GC_TRADE_VERIFY_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
// 에러 코드
////////////////////////////////////////////////////////////////////////////////

enum {
    // 교환 대상으로 아이템을 추가하는 것을 인증
    GC_TRADE_VERIFY_CODE_ADD_ITEM_WHEN_ACCEPT,

    // 교환 대상에서 아이템을 제거하는 것을 인증
    GC_TRADE_VERIFY_CODE_REMOVE_ITEM,

    // 교환 대상에서 돈을 추가하는 것을 인증
    GC_TRADE_VERIFY_CODE_MONEY_INCREASE,

    // 교환 대상에서 돈을 제거하는 것을 인증
    GC_TRADE_VERIFY_CODE_MONEY_DECREASE,

    // 교환 성립을 확인하는 것을 인증
    GC_TRADE_VERIFY_CODE_FINISH_ACCEPT,

    // 교환 성립을 취소하는 것을 인증
    GC_TRADE_VERIFY_CODE_FINISH_REJECT,

    // 교환 성립을 재고려하는 것을 인증
    GC_TRADE_VERIFY_CODE_FINISH_RECONSIDER,

    // 교환을 하고 있을 때 마우스에 인벤토리로 아이템을 옮기는 것을 검증
    GC_TRADE_VERIFY_CODE_MOUSE_TO_INVENTORY_OK,
    GC_TRADE_VERIFY_CODE_MOUSE_TO_INVENTORY_FAIL,

    // 교환을 하고 있을 때 인벤토리에서 마우스로 아이템을 옮기는 것을 검증
    GC_TRADE_VERIFY_CODE_INVENTORY_TO_MOUSE_OK,
    GC_TRADE_VERIFY_CODE_INVENTORY_TO_MOUSE_FAIL,

    // 현재로서는 선물 상자인 경우...
    GC_TRADE_VERIFY_CODE_ADD_ITEM_OK,
    GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL,

    // 에러닷.
    GC_TRADE_VERIFY_CODE_MAX
};

////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeVerify;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeVerify : public Packet {
public:
    GCTradeVerify(){};
    ~GCTradeVerify(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_TRADE_VERIFY;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }
    string getPacketName() const {
        return "GCTradeVerify";
    }
    string toString() const;

public:
    BYTE getCode() const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

private:
    BYTE m_Code; // 코드
};


////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeVerifyFactory;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeVerifyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_TRADE_VERIFY;
    static constexpr std::string_view kName = "GCTradeVerify";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    Packet* createPacket() override {
        return new GCTradeVerify();
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


////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////

#endif
