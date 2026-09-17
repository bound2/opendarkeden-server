#ifndef __NICKNAME_INFO_H__
#define __NICKNAME_INFO_H__

#include <string>

#include "Packet.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Types.h"

#define MAX_NICKNAME_SIZE 22

class NicknameInfo {
public:
    enum {
        NICK_NONE = 0,      // No nickname
        NICK_BUILT_IN,      // Nickname given automatically in the normal way (index)
        NICK_QUEST,         // Nickname received after clearing a quest (index)
        NICK_FORCED,        // Nickname assigned by force (index)
        NICK_CUSTOM_FORCED, // Nickname assigned by force (string)
        NICK_CUSTOM,        // Nickname the user entered freely (string)
    };

    NicknameInfo() : m_NicknameID(0), m_NicknameType(NICK_NONE), m_NicknameIndex(0) {}

    PacketSize_t getSize() const;
    static constexpr PacketSize_t getMaxSize() {
        return szWORD + szBYTE + szBYTE + MAX_NICKNAME_SIZE;
    }

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    WORD getNicknameID() const {
        return m_NicknameID;
    }
    BYTE getNicknameType() const {
        return m_NicknameType;
    }
    string getNickname() const {
        return m_Nickname;
    }
    WORD getNicknameIndex() const {
        return m_NicknameIndex;
    }

    void setNicknameID(WORD id) {
        m_NicknameID = id;
    }
    void setNicknameType(BYTE type) {
        m_NicknameType = type;
    }
    void setNickname(const string& name) {
        m_Nickname = (name.size() > MAX_NICKNAME_SIZE) ? name.substr(0, MAX_NICKNAME_SIZE) : name;
    }
    void setNicknameIndex(WORD index) {
        m_NicknameIndex = index;
    }

private:
    WORD m_NicknameID;
    BYTE m_NicknameType;
    string m_Nickname;
    WORD m_NicknameIndex; // Only one of the string and the index is used.
};

#endif
