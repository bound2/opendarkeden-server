#ifndef __PLAYER_FIXTURES_H__
#define __PLAYER_FIXTURES_H__

#include <string>

#include "DB.h"
#include "repository/StashRepository.h"

// SQL helpers for the MySQL integration tier: direct statements on the
// same connection the repositories use, for seeding and row inspection.

inline void execSQL(const std::string& sql) {
    Statement* pStmt = NULL;
    BEGIN_DB {
        pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
        pStmt->executeQueryString(sql);
        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt)
}

inline std::string queryScalar(const std::string& sql) {
    std::string value;
    Statement* pStmt = NULL;
    BEGIN_DB {
        pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
        Result* pResult = pStmt->executeQueryString(sql);
        if (pResult->next())
            value = pResult->getString(1);
        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt)
    return value;
}

// Ready-made character profiles for the integration tests: a low / mid /
// high level character of each race. NOTHING is persisted up front — each
// test picks the profiles it wants and calls persist() itself; a profile
// that is never persisted doubles as the "character with no rows" case.
//
// persist() writes the rows the way character creation does
// (CLCreatePCHandler): a Slayer row for EVERY race, then the race's own
// row — Vampire for slayers and vampires, Ousters for ousters. That shape
// is what gives the stash writes their Slayer-unconditional quirk.
// "Level" lands where each race keeps it: Vampire/Ousters have a Level
// column; slayers level per skill domain, so the profile level goes to
// SwordLevel.
struct PlayerFixture {
    std::string name; // fits every OwnerID/Name column (varchar(10))
    CharacterRace race;
    int level;

    void persist() const {
        char sql[160];
        if (race == CHARACTER_RACE_SLAYER) {
            sprintf(sql, "INSERT INTO Slayer (Name, SwordLevel) VALUES ('%s', %d)", name.c_str(), level);
            execSQL(sql);
            sprintf(sql, "INSERT INTO Vampire (Name) VALUES ('%s')", name.c_str());
            execSQL(sql);
        } else if (race == CHARACTER_RACE_VAMPIRE) {
            sprintf(sql, "INSERT INTO Slayer (Name) VALUES ('%s')", name.c_str());
            execSQL(sql);
            sprintf(sql, "INSERT INTO Vampire (Name, Level) VALUES ('%s', %d)", name.c_str(), level);
            execSQL(sql);
        } else {
            sprintf(sql, "INSERT INTO Slayer (Name) VALUES ('%s')", name.c_str());
            execSQL(sql);
            sprintf(sql, "INSERT INTO Ousters (Name, Level) VALUES ('%s', %d)", name.c_str(), level);
            execSQL(sql);
        }
    }

    void remove() const {
        execSQL("DELETE FROM Slayer WHERE Name = '" + name + "'");
        execSQL("DELETE FROM Vampire WHERE Name = '" + name + "'");
        execSQL("DELETE FROM Ousters WHERE Name = '" + name + "'");
    }
};

struct PlayerFixtures {
    static PlayerFixture lowLevelSlayer() {
        return make("itslaylo", CHARACTER_RACE_SLAYER, 5);
    }
    static PlayerFixture midLevelSlayer() {
        return make("itslaymid", CHARACTER_RACE_SLAYER, 75);
    }
    static PlayerFixture highLevelSlayer() {
        return make("itslayhi", CHARACTER_RACE_SLAYER, 150);
    }
    static PlayerFixture lowLevelVampire() {
        return make("itvamplo", CHARACTER_RACE_VAMPIRE, 5);
    }
    static PlayerFixture midLevelVampire() {
        return make("itvampmid", CHARACTER_RACE_VAMPIRE, 75);
    }
    static PlayerFixture highLevelVampire() {
        return make("itvamphi", CHARACTER_RACE_VAMPIRE, 150);
    }
    static PlayerFixture lowLevelOusters() {
        return make("itoustlo", CHARACTER_RACE_OUSTERS, 5);
    }
    static PlayerFixture midLevelOusters() {
        return make("itoustmid", CHARACTER_RACE_OUSTERS, 75);
    }
    static PlayerFixture highLevelOusters() {
        return make("itousthi", CHARACTER_RACE_OUSTERS, 150);
    }

    // Wipe every profile's rows — cheap enough to run in every SetUp so a
    // failed earlier run never leaks state into the next.
    static void removeAll() {
        const char* names = "('itslaylo', 'itslaymid', 'itslayhi', 'itvamplo', 'itvampmid', 'itvamphi', "
                            "'itoustlo', 'itoustmid', 'itousthi')";
        execSQL(std::string("DELETE FROM Slayer WHERE Name IN ") + names);
        execSQL(std::string("DELETE FROM Vampire WHERE Name IN ") + names);
        execSQL(std::string("DELETE FROM Ousters WHERE Name IN ") + names);
    }

private:
    static PlayerFixture make(const char* name, CharacterRace race, int level) {
        PlayerFixture fixture;
        fixture.name = name;
        fixture.race = race;
        fixture.level = level;
        return fixture;
    }
};

#endif
