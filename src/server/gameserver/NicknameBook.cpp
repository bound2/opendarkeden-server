#include "NicknameBook.h"

#include "GCNicknameList.h"
#include "Guild.h"
#include "GuildManager.h"
#include "LevelNickInfoManager.h"
#include "NicknameInfo.h"
#include "PlayerCreature.h"
#include "repository/NicknameRepository.h"

#define CUSTOM_NICKNAME_ID 0
#define LEVEL_NICKNAME_BASE_ID 1
#define GUILD_MASTER_NICKNAME_ID 11

// m_NextNicknameID starts at 10000 unconditionally, exactly as the inline
// SQL version set it before opening its DB block: if load()'s query throws,
// popNicknameID() must still hand out well-defined ids.
NicknameBook::NicknameBook(PlayerCreature* pOwner, NicknameRepository* pRepository)
    : m_pOwner(pOwner), m_pRepository(pRepository != 0 ? pRepository : &defaultNicknameRepository()),
      m_NextNicknameID(10000) {}

void NicknameBook::load() {
    __BEGIN_TRY

    vector<NicknameRecord> records = m_pRepository->load(m_pOwner->getName());
    m_NextNicknameID = nextNicknameIDAfter(records);

    for (size_t i = 0; i < records.size(); ++i) {
        const NicknameRecord& record = records[i];

        if (record.type != NicknameInfo::NICK_NONE) {
            NicknameInfo* pNickname = new NicknameInfo;
            pNickname->setNicknameID(record.id);
            pNickname->setNicknameType(record.type);
            pNickname->setNickname(record.nickname);
            pNickname->setNicknameIndex(record.index);

            setNicknameInfo(record.id, pNickname);

            if (record.type == NicknameInfo::NICK_FORCED || record.type == NicknameInfo::NICK_CUSTOM_FORCED) {
                m_pOwner->setNickname(pNickname);
            }
        }
    }

    if (!hasCustomSlot(records)) {
        NicknameInfo* pLevelNickname = new NicknameInfo;
        pLevelNickname->setNicknameID(CUSTOM_NICKNAME_ID);
        pLevelNickname->setNicknameType(NicknameInfo::NICK_CUSTOM);
        pLevelNickname->setNickname(" ");
        setNicknameInfo(CUSTOM_NICKNAME_ID, pLevelNickname);

        m_pRepository->insertDefaultCustomSlot(m_pOwner->getName());
    }

    /*	if ( m_pOwner->getLevel() >= 10 )
        {
            Level_t level = m_pOwner->getLevel();
            if ( level > 150 ) level=150;
            NicknameInfo* pLevelNickname = new NicknameInfo;
            pLevelNickname->setNicknameID( 1 );
            pLevelNickname->setNicknameType( NicknameInfo::NICK_BUILT_IN );
            pLevelNickname->setNicknameIndex( level/10 );
            setNicknameInfo( 1, pLevelNickname );

            cout << "닉네임 번호 : " << level/10 << endl;
        }*/

    if (m_pOwner->getLevel() >= 10) {
        Level_t level = m_pOwner->getLevel();
        vector<LevelNickInfo*>& infos = LevelNickInfoManager::Instance().getLevelNickInfo(level);
        vector<LevelNickInfo*>::iterator itr = infos.begin();
        vector<LevelNickInfo*>::iterator endItr = infos.end();
        int nid = LEVEL_NICKNAME_BASE_ID;

        for (; itr != endItr; ++itr) {
            if (!(*itr)->isFitRace(m_pOwner))
                continue;

            NicknameInfo* pLevelNickname = new NicknameInfo;
            pLevelNickname->setNicknameID(nid);
            pLevelNickname->setNicknameType(NicknameInfo::NICK_BUILT_IN);
            pLevelNickname->setNicknameIndex((*itr)->getNickIndex());
            setNicknameInfo(nid++, pLevelNickname);
        }
    }

    if (m_pOwner->getGuildID() != m_pOwner->getCommonGuildID()) {
        // Fetch the guild.
        Guild* pGuild = g_pGuildManager->getGuild(m_pOwner->getGuildID());

        // Check whether the player is the guild's master.
        if (pGuild != NULL && pGuild->getMaster() == m_pOwner->getName()) {
            NicknameInfo* pLevelNickname = new NicknameInfo;
            pLevelNickname->setNicknameID(GUILD_MASTER_NICKNAME_ID);
            pLevelNickname->setNicknameType(NicknameInfo::NICK_BUILT_IN);
            pLevelNickname->setNicknameIndex(47);
            setNicknameInfo(GUILD_MASTER_NICKNAME_ID, pLevelNickname);
        }
    }

    __END_CATCH
}

Packet* NicknameBook::getNicknameBookListPacket() const {
    GCNicknameList* pPacket = new GCNicknameList;

    vector<NicknameInfo*>& nickList = pPacket->getNicknames();

    unordered_map<WORD, NicknameInfo*>::const_iterator itr = m_Nicknames.begin();
    unordered_map<WORD, NicknameInfo*>::const_iterator endItr = m_Nicknames.end();

    for (; itr != endItr; ++itr) {
        if (itr->second != NULL)
            nickList.push_back(itr->second);
    }

    return pPacket;
}

void NicknameBook::addNewNickname(const string& nick)

{
    __BEGIN_TRY

    NicknameInfo* pNickname = new NicknameInfo;
    pNickname->setNicknameID(popNicknameID());
    pNickname->setNicknameType(NicknameInfo::NICK_CUSTOM);
    pNickname->setNickname(nick);

    setNicknameInfo(pNickname->getNicknameID(), pNickname);

    m_pRepository->insert(m_pOwner->getName(), pNickname->getNicknameID(), pNickname->getNicknameType(),
                          pNickname->getNickname());

    __END_CATCH
}
