#ifndef __GUILD_UNION_H__
#define __GUILD_UNION_H__

#include <list>

#include <unordered_map>

#include "Exception.h"
#include "GCUnionOfferList.h"
#include "Mutex.h"
#include "Types.h"

class GuildUnionManager;
class GuildUnion {
public:
    GuildUnion(GuildID_t master) : m_MasterGuildID(master) {}
    ~GuildUnion();

    GuildID_t getMasterGuildID() const {
        return m_MasterGuildID;
    }

    uint getUnionID() const {
        return m_UnionID;
    }
    void setUnionID(uint ID) {
        m_UnionID = ID;
    }

    bool hasGuild(GuildID_t gID) const;
    bool addGuild(GuildID_t gID);
    bool removeGuild(GuildID_t gId);

    void create();
    void destroy();

    list<GuildID_t> getGuildList() const {
        return m_Guilds;
    }

protected:
    list<GuildID_t>::const_iterator findGuildItr(GuildID_t gID) const {
        list<GuildID_t>::const_iterator itr = m_Guilds.begin();
        for (; itr != m_Guilds.end(); itr++) {
            if (*itr == gID) {
                break;
            }
        }
        return itr;
    }
    list<GuildID_t>::iterator findGuildItr(GuildID_t gID) {
        // return find( m_Guilds.begin(), m_Guilds.end(), gID );
        list<GuildID_t>::iterator itr = m_Guilds.begin();
        for (; itr != m_Guilds.end(); itr++) {
            if (*itr == gID) {
                break;
            }
        }
        return itr;
    }

private:
    uint m_UnionID;
    GuildID_t m_MasterGuildID;
    list<GuildID_t> m_Guilds;

    friend class GuildUnionManager;
};

class GuildUnionManager {
public:
    GuildUnionManager();
    ~GuildUnionManager();

    void reload();
    void load();
    void addGuildUnion(GuildUnion* pUnion);
    // The union holding this guild, master or member, or NULL for a guild in
    // none. A lookup only reads: an id the map does not hold leaves no entry
    // behind.
    GuildUnion* getGuildUnion(GuildID_t gID) const {
        unordered_map<GuildID_t, GuildUnion*>::const_iterator itr = m_GuildUnionMap.find(gID);
        return itr == m_GuildUnionMap.end() ? NULL : itr->second;
    }
    GuildUnion* getGuildUnionByUnionID(uint uID) const {
        unordered_map<uint, GuildUnion*>::const_iterator itr = m_UnionIDMap.find(uID);
        return itr == m_UnionIDMap.end() ? NULL : itr->second;
    }

    bool addGuild(uint uID, GuildID_t gID);

    // Settle a guild's union standing because the guild is going away. A
    // member guild leaves and the union carries on; a master guild takes the
    // union with it, since mastery cannot be handed on. The rule is
    // decideUnionTeardown's (guild/GuildUnionTeardown.h); this performs it
    // over the union tables, the lookup maps and the guilds still online.
    // Call it while the guild is still in the GuildManager: the guild
    // masters it notifies are read from there, and a guild that has already
    // gone is logged to GuildUnion.log and skipped.
    bool removeGuildFromUnion(GuildID_t gID);

    bool removeGuild(uint uID, GuildID_t gID);

    void sendRefreshCommand();
    void sendModifyUnionInfo(uint gID);

    static GuildUnionManager& Instance() {
        static GuildUnionManager theInstance;
        return theInstance;
    }

private:
    // Destroy a union: its own row and its member rows go, every guild that
    // looked it up stops finding it, and the object is freed.
    void destroyUnion(uint uID);

    list<GuildUnion*> m_GuildUnionList;
    unordered_map<GuildID_t, GuildUnion*> m_GuildUnionMap;
    unordered_map<uint, GuildUnion*> m_UnionIDMap;


    // Mutex
    mutable Mutex m_Mutex;
};

class GuildUnionOffer {
public:
    enum OfferType { JOIN, QUIT };
    BYTE m_Type;
    GuildID_t m_GuildID;
    string m_MasterID;
    string m_GuildName;
    string m_MasterName;
};

class GuildUnionOfferManager {
public:
    enum ErrorType {
        OK = 0,
        ALREADY_IN_UNION,
        ALREADY_OFFER_SOMETHING,
        TARGET_IS_NOT_MASTER, // The other side is not a master
        NOT_IN_UNION,
        MASTER_CANNOT_QUIT, // A union master guild cannot leave on its own (automatic once no sub-guild is left)
        NO_TARGET_UNION,
        NOT_YOUR_UNION,
        SOURCE_IS_NOT_MASTER, // The applicant is not a master
        YOU_HAVE_PENALTY,     // Has a record of a forced withdrawal and cannot apply
        NOT_ENOUGH_SLOT,      // On OfferJoin: the joinable slots are full
        TOO_MANY_MEMBER       // Cannot join because there are 50 or more members


    };
    uint offerJoin(GuildID_t gID, GuildID_t MasterGID); // Apply to join a union
    uint offerQuit(GuildID_t gID);                      // Apply to leave a union

    uint acceptJoin(GuildID_t gID); // Accept a union join
    uint acceptQuit(GuildID_t gID); // Accept a union withdrawal

    uint denyJoin(GuildID_t gID); // Deny a union join
    uint denyQuit(GuildID_t gID); // Deny a union withdrawal

    void clearOffer(GuildID_t gID);
    bool hasOffer(GuildID_t gID);

    bool makeOfferList(uint uID, GCUnionOfferList& offerList); // Request the list

    static GuildUnionOfferManager& Instance() {
        static GuildUnionOfferManager theInstance;
        return theInstance;
    }
};

#endif
