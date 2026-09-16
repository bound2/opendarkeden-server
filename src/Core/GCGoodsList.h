//////////////////////////////////////////////////////////////////////////////
// Filename    : GCGoodsList.h
// Written By  : 김성민
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_GOODS_LIST_H__
#define __GC_GOODS_LIST_H__

#include <list>

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "SubItemInfo.h"

#define MAX_GOODS_LIST 20

typedef struct _GoodsInfo {
    int getPacketSize() const {
        return szObjectID + szBYTE + szItemType + szGrade + szBYTE + optionType.size() + szItemNum + szDWORD;
    }

    static constexpr int getPacketMaxSize() {
        return szObjectID + szBYTE + szItemType + szGrade + szBYTE + 255 + szItemNum + szDWORD;
    }

    string toString() const {
        StringStream msg;
        msg << "Good(" << "ObjectID : " << objectID << ", ItemClass : " << (int)itemClass << ", ItemType : " << itemType
            << ", Grade : " << grade << ", Options : (";

        list<OptionType_t>::const_iterator itr = optionType.begin();
        list<OptionType_t>::const_iterator endItr = optionType.end();

        for (; itr != endItr; ++itr) {
            msg << *itr << ", ";
        }

        msg << "), Num : " << num << ", TimeLimit : " << timeLimit;

        return msg.toString();
    }

    ObjectID_t objectID;
    BYTE itemClass;
    ItemType_t itemType;
    Grade_t grade;
    list<OptionType_t> optionType;
    ItemNum_t num;
    DWORD timeLimit;
} GoodsInfo;

//////////////////////////////////////////////////////////////////////////////
// class GCGoodsList;
//////////////////////////////////////////////////////////////////////////////

class Item;

class GCGoodsList : public Packet {
public:
    GCGoodsList();
    virtual ~GCGoodsList();

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_GOODS_LIST;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCGoodsList";
    }
    string toString() const;

public:
    // The options one record carries. The count travels in a BYTE and
    // GoodsInfo::getPacketMaxSize budgets this many.
    static constexpr size_t kMaxOptionCount = 255;

    // The packet owns every record it holds and frees it.
    void addGoodsInfo(GoodsInfo* pGI) {
        if (pGI == NULL)
            throw InvalidProtocolException("GCGoodsList : null goods record");
        if (m_GoodsList.size() >= (size_t)MAX_GOODS_LIST)
            throw InvalidProtocolException("GCGoodsList : too many goods records");
        if (pGI->optionType.size() > kMaxOptionCount)
            throw InvalidProtocolException("GCGoodsList : too many record options");
        m_GoodsList.push_back(pGI);
    }
    // Hands the record over to the caller, which frees it.
    GoodsInfo* popGoodsInfo() {
        if (m_GoodsList.empty())
            throw InvalidProtocolException("GCGoodsList : no goods record to pop");
        GoodsInfo* pRet = m_GoodsList.front();
        m_GoodsList.pop_front();
        return pRet;
    }

private:
    // Free every record the packet holds.
    void clearGoodsInfo();

    list<GoodsInfo*> m_GoodsList;
};


//////////////////////////////////////////////////////////////////////////////
// class GCGoodsListFactory;
//////////////////////////////////////////////////////////////////////////////

class GCGoodsListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GOODS_LIST;
    static constexpr std::string_view kName = "GCGoodsList";
    static constexpr PacketSize_t kMaxSize{[] {
        PacketSize_t size = szBYTE;
        size += GoodsInfo::getPacketMaxSize() * MAX_GOODS_LIST;

        return size;
    }()};

    Packet* createPacket() override {
        return new GCGoodsList();
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
