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

void GuildUnion::replaceMembers(const std::list<GuildID_t>& memberGuilds) {
    __ENTER_CRITICAL_SECTION(m_Mutex)

    m_Guilds = memberGuilds;

    __LEAVE_CRITICAL_SECTION(m_Mutex)
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

    // The live unions by id. Only the first of two live unions with one id
    // can be kept; the other is retired below with the rest left over.
    std::unordered_map<uint, size_t> previous;
    for (size_t u = 0; u < m_Live.size(); u++)
        previous.emplace(m_Live[u]->getUnionID(), u);

    std::vector<std::unique_ptr<GuildUnion>> live;
    live.reserve(fresh.size());

    for (size_t u = 0; u < fresh.size(); u++) {
        std::unordered_map<uint, size_t>::iterator itr = previous.find(fresh[u]->getUnionID());

        if (itr != previous.end() && m_Live[itr->second]->getMasterGuildID() == fresh[u]->getMasterGuildID()) {
            // Kept: the fresh copy lends its member list and is dropped
            // with `fresh`, never having been handed out.
            m_Live[itr->second]->replaceMembers(fresh[u]->getGuildList());
            live.push_back(std::move(m_Live[itr->second]));
            previous.erase(itr);
        } else {
            live.push_back(std::move(fresh[u]));
        }
    }

    // What is left of the old set has vanished from the tables or changed
    // master.
    for (size_t u = 0; u < m_Live.size(); u++) {
        if (m_Live[u] != nullptr) {
            m_Live[u]->retire();
            m_Retired.push_back(std::move(m_Live[u]));
        }
    }

    m_Live = std::move(live);
    m_ByGuild.clear();
    m_ByID.clear();

    for (size_t u = 0; u < m_Live.size(); u++)
        publish_LOCKED(m_Live[u].get());

    __LEAVE_CRITICAL_SECTION(m_Mutex)
}

size_t GuildUnionRegistry::retiredCount() const {
    size_t count = 0;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    count = m_Retired.size();

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return count;
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
