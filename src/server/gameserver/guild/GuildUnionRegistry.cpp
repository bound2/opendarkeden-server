//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildUnionRegistry.cpp
//////////////////////////////////////////////////////////////////////////////

#include "GuildUnionRegistry.h"

#include <algorithm>
#include <utility>

GuildUnion::GuildUnion(uint unionID, GuildID_t masterGuildID, const std::list<GuildID_t>& memberGuilds)
    : m_UnionID(unionID), m_MasterGuildID(masterGuildID), m_Guilds(memberGuilds) {
    m_Mutex.setName("GuildUnion");
}

bool GuildUnion::hasGuild(GuildID_t gID) const {
    bool found = false;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    if (!isRetired())
        found = gID == m_MasterGuildID || std::find(m_Guilds.begin(), m_Guilds.end(), gID) != m_Guilds.end();

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return found;
}

std::list<GuildID_t> GuildUnion::getGuildList() const {
    std::list<GuildID_t> guilds;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    if (!isRetired())
        guilds = m_Guilds;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return guilds;
}

bool GuildUnion::addMember(GuildID_t gID) {
    bool added = false;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    if (!isRetired() && gID != m_MasterGuildID && std::find(m_Guilds.begin(), m_Guilds.end(), gID) == m_Guilds.end()) {
        m_Guilds.push_back(gID);
        added = true;
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return added;
}

bool GuildUnion::removeMember(GuildID_t gID) {
    bool removed = false;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    std::list<GuildID_t>::iterator itr = std::find(m_Guilds.begin(), m_Guilds.end(), gID);
    if (!isRetired() && itr != m_Guilds.end()) {
        m_Guilds.erase(itr);
        removed = true;
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return removed;
}

void GuildUnion::retire() {
    __ENTER_CRITICAL_SECTION(m_Mutex)

    m_Retired.store(true);

    __LEAVE_CRITICAL_SECTION(m_Mutex)
}

GuildUnionRegistry::GuildUnionRegistry() {
    m_Mutex.setName("GuildUnionRegistry");
}

// Nothing can hold a union past the registry: it lives as long as the
// process's union manager.
GuildUnionRegistry::~GuildUnionRegistry() = default;

GuildUnion* GuildUnionRegistry::unionOfGuild(GuildID_t gID) const {
    GuildUnion* pUnion = NULL;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    std::unordered_map<GuildID_t, GuildUnion*>::const_iterator itr = m_ByGuild.find(gID);
    if (itr != m_ByGuild.end())
        pUnion = itr->second;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return pUnion;
}

GuildUnion* GuildUnionRegistry::unionByID(uint uID) const {
    GuildUnion* pUnion = NULL;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    std::unordered_map<uint, GuildUnion*>::const_iterator itr = m_ByID.find(uID);
    if (itr != m_ByID.end())
        pUnion = itr->second;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return pUnion;
}

GuildUnion* GuildUnionRegistry::publish(std::unique_ptr<GuildUnion> pUnion) {
    GuildUnion* pPublished = pUnion.get();

    __ENTER_CRITICAL_SECTION(m_Mutex)

    m_Live.push_back(std::move(pUnion));
    publish_LOCKED(pPublished);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return pPublished;
}

bool GuildUnionRegistry::addMember(uint uID, GuildID_t gID) {
    bool added = false;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    std::unordered_map<uint, GuildUnion*>::iterator itr = m_ByID.find(uID);
    if (itr != m_ByID.end() && m_ByGuild.find(gID) == m_ByGuild.end() && itr->second->addMember(gID)) {
        m_ByGuild[gID] = itr->second;
        added = true;
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return added;
}

bool GuildUnionRegistry::removeMember(uint uID, GuildID_t gID) {
    bool removed = false;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    std::unordered_map<uint, GuildUnion*>::iterator itr = m_ByID.find(uID);
    if (itr != m_ByID.end() && itr->second->removeMember(gID)) {
        std::unordered_map<GuildID_t, GuildUnion*>::iterator guildItr = m_ByGuild.find(gID);
        if (guildItr != m_ByGuild.end() && guildItr->second == itr->second)
            m_ByGuild.erase(guildItr);
        removed = true;
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return removed;
}

bool GuildUnionRegistry::retire(uint uID) {
    bool retired = false;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    std::unordered_map<uint, GuildUnion*>::iterator itr = m_ByID.find(uID);
    if (itr != m_ByID.end()) {
        retire_LOCKED(itr->second);
        retired = true;
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return retired;
}

void GuildUnionRegistry::replaceAll(std::vector<std::unique_ptr<GuildUnion>> fresh) {
    __ENTER_CRITICAL_SECTION(m_Mutex)

    for (size_t u = 0; u < m_Live.size(); u++) {
        m_Live[u]->retire();
        m_Retired.push_back(std::move(m_Live[u]));
    }
    m_Live.clear();
    m_ByGuild.clear();
    m_ByID.clear();

    for (size_t u = 0; u < fresh.size(); u++) {
        GuildUnion* pUnion = fresh[u].get();
        m_Live.push_back(std::move(fresh[u]));
        publish_LOCKED(pUnion);
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)
}

void GuildUnionRegistry::publish_LOCKED(GuildUnion* pUnion) {
    m_ByID[pUnion->getUnionID()] = pUnion;
    m_ByGuild[pUnion->getMasterGuildID()] = pUnion;

    const std::list<GuildID_t> members = pUnion->getGuildList();
    for (std::list<GuildID_t>::const_iterator itr = members.begin(); itr != members.end(); ++itr)
        m_ByGuild[*itr] = pUnion;
}

// Takes the union out of both tables -- only the entries that still name it,
// since a later union may have taken over an id or a guild -- and parks it.
void GuildUnionRegistry::retire_LOCKED(GuildUnion* pUnion) {
    const std::list<GuildID_t> members = pUnion->getGuildList();
    for (std::list<GuildID_t>::const_iterator itr = members.begin(); itr != members.end(); ++itr) {
        std::unordered_map<GuildID_t, GuildUnion*>::iterator guildItr = m_ByGuild.find(*itr);
        if (guildItr != m_ByGuild.end() && guildItr->second == pUnion)
            m_ByGuild.erase(guildItr);
    }

    std::unordered_map<GuildID_t, GuildUnion*>::iterator masterItr = m_ByGuild.find(pUnion->getMasterGuildID());
    if (masterItr != m_ByGuild.end() && masterItr->second == pUnion)
        m_ByGuild.erase(masterItr);

    std::unordered_map<uint, GuildUnion*>::iterator idItr = m_ByID.find(pUnion->getUnionID());
    if (idItr != m_ByID.end() && idItr->second == pUnion)
        m_ByID.erase(idItr);

    pUnion->retire();

    for (size_t u = 0; u < m_Live.size(); u++) {
        if (m_Live[u].get() == pUnion) {
            m_Retired.push_back(std::move(m_Live[u]));
            m_Live.erase(m_Live.begin() + u);
            break;
        }
    }
}
