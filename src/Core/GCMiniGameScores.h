//////////////////////////////////////////////////////////////////////////////
// Filename    : GCMiniGameScores.h
// Written By  : elca@ewestsoft.com
// Description :
// Class definition for the packet sent when a skill succeeds
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_MINI_GAME_SCORES_H__
#define __GC_MINI_GAME_SCORES_H__

#include <list>

#include "Assert1.h"
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"
#include "WireString.h"

enum GameType { GAME_MINE = 0, GAME_NEMO, GAME_PUSH, GAME_ARROW };

//////////////////////////////////////////////////////////////////////////////
// class GCMiniGameScores;
// Class the game server uses to tell the client that its own skill succeeded
//////////////////////////////////////////////////////////////////////////////

class GCMiniGameScores : public Packet {
public:
    // The entries write() emits and the name width the factory max
    // budgets for each.
    static constexpr uint kMaxScores = 10;
    static constexpr uint kMaxNameLength = 20;

    GCMiniGameScores();
    ~GCMiniGameScores();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_MINI_GAME_SCORES;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCMiniGameScores";
    }
    string toString() const;

public:
    BYTE getGameType() const {
        return m_GameType;
    }
    void setGameType(GameType type) {
        m_GameType = (BYTE)type;
    }

    BYTE getLevel() const {
        return m_Level;
    }
    void setLevel(BYTE level) {
        m_Level = (BYTE)level;
    }

    const pair<string, WORD> popScore() {
        pair<string, WORD> ret = m_Scores.front();
        m_Scores.pop_front();
        return ret;
    }
    // The name is cut to the width the factory max budgets.
    void addScore(const string& name, WORD score) {
        m_Scores.push_back(pair<string, WORD>(name.substr(0, kMaxNameLength), score));
    }
    list<pair<string, WORD>>::size_type getSize() const {
        return m_Scores.size();
    }

private:
    BYTE m_GameType = 0;
    BYTE m_Level = 0;
    list<pair<string, WORD>> m_Scores;
};


//////////////////////////////////////////////////////////////////////////////
// class GCMiniGameScoresFactory;
//////////////////////////////////////////////////////////////////////////////

class GCMiniGameScoresFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MINI_GAME_SCORES;
    static constexpr std::string_view kName = "GCMiniGameScores";
    static constexpr PacketSize_t kMaxSize{
        szBYTE + szBYTE + szBYTE + (szWORD + szBYTE + GCMiniGameScores::kMaxNameLength) * GCMiniGameScores::kMaxScores};

    GCMiniGameScoresFactory() {}
    virtual ~GCMiniGameScoresFactory() {}

public:
    Packet* createPacket() override {
        return new GCMiniGameScores();
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
