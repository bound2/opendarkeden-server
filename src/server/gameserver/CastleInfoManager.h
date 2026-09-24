#ifndef __CASTLE_INFO_MANAGER_H__
#define __CASTLE_INFO_MANAGER_H__

#include <atomic>

#include <unordered_map>

#include "CastleTaxBalance.h"
#include "CommonGuild.h"
#include "Exception.h"
#include "PlayerCreature.h"
#include "Types.h"

class Zone;
class NPC;

class CastleInfo {
public:
    enum ResurrectPriority {
        CASTLE_RESURRECT_PRIORITY_FIRST,
        CASTLE_RESURRECT_PRIORITY_SECOND,
        CASTLE_RESURRECT_PRIORITY_THIRD,

        CASTLE_RESURRECT_PRIORITY_MAX
    };

public:
    CastleInfo();
    ~CastleInfo();

public:
    ZoneID_t getZoneID() const {
        return m_ZoneID;
    }
    void setZoneID(const ZoneID_t zoneID) {
        m_ZoneID = zoneID;
    }

    ShrineID_t getShrineID() const {
        return m_ShrineID;
    }
    void setShrineID(const ShrineID_t id) {
        m_ShrineID = id;
    }

    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(const GuildID_t guildID) {
        m_GuildID = guildID;
    }

    const string& getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    int getItemTaxRatio() const {
        return m_ItemTaxRatio;
    }
    void setItemTaxRatio(const int itemTaxRatio) {
        m_ItemTaxRatio = itemTaxRatio;
    }

    Gold_t getEntranceFee() const {
        return m_EntranceFee;
    }
    void setEntranceFee(const Gold_t entranceFee) {
        m_EntranceFee = entranceFee;
    }

    // The balance is changed only through CastleInfoManager, which saves
    // every change it makes (see increaseTaxBalance there).
    Gold_t getTaxBalance() const {
        return m_TaxBalance;
    }

    const list<OptionType_t>& getOptionTypeList() const {
        return m_BonusOptionList;
    }
    void setOptionTypeList(const string& options);

    bool isCastleZone(ZoneID_t targetZoneID) const;
    const list<ZoneID_t>& getZoneIDList() const {
        return m_CastleZoneIDList;
    }
    void setZoneIDList(const string& zoneIDs);

    Race_t getRace() const {
        return m_Race;
    }
    void setRace(Race_t race) {
        m_Race = race;
    }

    void setResurrectPosition(ResurrectPriority resurrectPriority, const ZONE_COORD& zoneCoord);
    void getResurrectPosition(ResurrectPriority resurrectPriority, ZONE_COORD& zoneCoord);

    bool isCommon() const {
        return isCommonGuildID(m_GuildID);
    }

    void broadcast(Packet* pPacket) const;

    string toString() const;

private:
    friend class CastleInfoManager;

    // The load's value, before any thread runs.
    void setTaxBalance(const Gold_t balance) {
        m_TaxBalance = balance;
    }
    TaxBalanceChange increaseTaxBalance(Gold_t tax) {
        return creditTaxBalance(m_TaxBalance, tax);
    }
    TaxBalanceChange decreaseTaxBalance(Gold_t tax) {
        return debitTaxBalance(m_TaxBalance, tax);
    }
    TaxBalanceChange takeTaxBalance() {
        return ::takeTaxBalance(m_TaxBalance);
    }

    // The zone, shrine, name, bonus options, zone list and resurrection
    // positions are loaded once and never written again. The owner and what
    // follows from it -- guild, race, entrance fee, item tax ratio -- change
    // only through CastleInfoManager::modifyCastleOwner and setItemTaxRatio
    // on the castle zone's own thread, while every zone thread reads them
    // without a lock (castle gates, shops, resurrection), so they are atomics:
    // a reader gets a value a writer stored, and two of them read one after
    // the other may straddle an owner change. The tax balance is a counter
    // that shops, resurrection fees and withdrawals move from any zone thread,
    // so it is changed by compare-and-swap rather than read and rewritten
    // (CastleTaxBalance.h).
    ZoneID_t m_ZoneID;                 // Zone ID
    ShrineID_t m_ShrineID;             // ShrineID of the castle symbol
    std::atomic<GuildID_t> m_GuildID;  // ID of the owning guild
    string m_Name;                     // Castle name
    std::atomic<int> m_ItemTaxRatio;   // Tax rate when buying items (%)
    std::atomic<Gold_t> m_EntranceFee; // Entrance fee
    std::atomic<Gold_t> m_TaxBalance;  // Tax accumulated so far
    std::atomic<Race_t> m_Race;        // Which race the castle belongs to

    list<OptionType_t> m_BonusOptionList; // Race bonus
    list<ZoneID_t> m_CastleZoneIDList;

    ZONE_COORD m_ResurrectPosition[CASTLE_RESURRECT_PRIORITY_MAX]; // Resurrection positions of the castle
};

class CastleInfoManager {
public:
    CastleInfoManager();
    ~CastleInfoManager();

public:
    void init();
    void load();

    void addCastleInfo(CastleInfo* pCastleInfo);
    void deleteCastleInfo(ZoneID_t zoneID);
    CastleInfo* getCastleInfo(ZoneID_t zoneID) const;
    int size() const {
        return m_CastleInfos.size();
    }

    // Hands the castle to a new owner: writes its row, resets the tax
    // balance (saved as the debit of what it held, see increaseTaxBalance),
    // entrance fee and item tax ratio, and, when the owning race
    // changes, cancels the castle's war schedules and reloads its scheduler,
    // which frees the wars the castle zone's thread executes. So it runs only
    // on that thread, under the group mutex: callers on other threads post it
    // there (ZoneGroup::post), as the two settlements below do.
    bool modifyCastleOwner(ZoneID_t zoneID, PlayerCreature* pPC);
    bool modifyCastleOwner(ZoneID_t zoneID, Race_t race, GuildID_t guildID);

    // Settles a castle war's end on the castle zone's group, from whichever
    // thread ended it: hands the castle to the winner when the war changed
    // its owner (to the winner race's common guild if the winning guild has
    // been deleted by then, see castleWarWinnerOwner), then credits the war's
    // registration fee to the castle's balance, after the change has reset
    // it. Captures values only; the war may be freed before the command runs.
    void postCastleWarEnd(ZoneID_t castleZoneID, bool bChangeOwner, Race_t winnerRace, GuildID_t winnerGuildID,
                          Gold_t registrationFee);

    // Turns the castle common if the deleted guild still holds it when this
    // runs (castleOwnerAfterGuildDeleted). Runs on the castle zone's thread,
    // posted there by the guild deletion.
    void settleGuildDeletion(ZoneID_t castleZoneID, GuildID_t deletedGuildID);

    bool tinysave(ZoneID_t zoneID, const string& query);

    // Move a castle's tax balance by up to `tax`, clamped to the balance's
    // range, and save the amount actually moved as a relative change of the
    // row (TaxBalance plus or minus it). Credits come from any zone thread
    // (shop taxes, entrance and resurrection fees, war registration fees);
    // debits only from the castle's own (a withdrawal). A zero change saves
    // nothing; an unknown castle changes nothing.
    //
    // The row and the memory are the load's value plus every applied change,
    // the owner change's reset included, so the order two threads' saves
    // land in does not matter: once each has landed the row is the balance.
    // In between, the row may briefly stand below zero or above the maximum
    // (a withdrawal saved ahead of the credit it drew on), which the signed
    // column holds and the load clamps. What remains is a save that never
    // lands -- a database error, or a crash between a change and its save --
    // which leaves the row off by that one change for good, where an
    // absolute save would have been corrected by the next change.
    TaxBalanceChange increaseTaxBalance(ZoneID_t zoneID, Gold_t tax);
    TaxBalanceChange decreaseTaxBalance(ZoneID_t zoneID, Gold_t tax);

    bool setItemTaxRatio(Zone* pZone, int itemTaxRatio);
    const unordered_map<ZoneID_t, CastleInfo*>& getCastleInfos() const {
        return m_CastleInfos;
    }

    int getItemTaxRatio(const PlayerCreature* pPC, const NPC* pNPC = NULL) const;
    Gold_t getEntranceFee(ZoneID_t zoneID, PlayerCreature* pPC) const;

    bool isCastleMember(PlayerCreature* pPC) const;
    bool isCastleMember(ZoneID_t zoneID, PlayerCreature* pPC) const;
    bool isPossibleEnter(ZoneID_t zoneID, PlayerCreature* pPC) const;
    bool canPortalActivate(ZoneID_t zoneID, PlayerCreature* pPC) const;
    bool hasOtherBloodBible(ZoneID_t zoneID, PlayerCreature* pPC) const;

    CastleInfo* getGuildCastleInfo(GuildID_t guildID) const;
    list<CastleInfo*> getGuildCastleInfos(GuildID_t guildID) const;

    bool getResurrectPosition(PlayerCreature* pPC, ZONE_COORD& zoneCoord);

    //----------------------------------------------------------------------
    // CastleZoneID related
    //----------------------------------------------------------------------
    bool isCastleZone(ZoneID_t castleZoneID, ZoneID_t targetZoneID) const;
    bool isCastleZone(ZoneID_t zoneID) const;
    void clearCastleZoneIDs();
    bool getCastleZoneID(ZoneID_t zoneID, ZoneID_t& castleZoneID) const;
    void setCastleZoneID(ZoneID_t zoneID, ZoneID_t castleZoneID);
    bool isSameCastleZone(ZoneID_t zoneID1, ZoneID_t zoneID2) const;

    //----------------------------------------------------------------------
    // Things that apply to every castle
    //----------------------------------------------------------------------
    void releaseAllSafeZone();
    void resetAllSafeZone();

    void deleteAllNPCs();
    void loadAllNPCs();

    void transportAllOtherRace();

    ZoneID_t getCastleZoneID(ShrineID_t shrineID) const;
    void broadcastShrinePacket(ShrineID_t shrineID, Packet* pPacket) const;

    SkillType_t getCastleSkillType(ZoneID_t zoneID, GuildID_t guildID) const;

    string toString() const;

private:
    void saveTaxBalanceChange(ZoneID_t zoneID, int64_t delta);

    unordered_map<ZoneID_t, CastleInfo*> m_CastleInfos;
    unordered_map<ZoneID_t, ZoneID_t> m_CastleZoneIDs;
};

#endif // __CASTlE_INFO_MANAGER_H__
