// RaceSkillSlot is what the vampire and ousters skill slots have in common.
// Two pieces of behaviour live there rather than in the race classes: the
// run-time arithmetic the client is told its cooldowns in, and the rule that
// a cast which changes a skill's delay writes the new delay back -- through
// the race's own save(), because only the race knows its table. A slot type
// whose save() stopped being reached would persist nothing and no caller
// would notice.

#include <gtest/gtest.h>

#include "skill/RaceSkillSlot.h"

namespace {

// A slot that records what it was asked to persist instead of writing a row.
class RecordingSkillSlot : public RaceSkillSlot {
public:
    RecordingSkillSlot() : RaceSkillSlot() {}
    RecordingSkillSlot(SkillType_t skillType, ulong interval, ulong castingTime)
        : RaceSkillSlot(skillType, interval, castingTime) {}

    void save(const string& ownerID) override {
        ++m_SaveCount;
        m_LastOwner = ownerID;
    }
    void save() override {
        ++m_SaveCount;
    }
    void create(const string&) override {}

    // The run time is normally read from the clock; the tests set it.
    void placeRunTime(long sec, long usec) {
        m_runTime.tv_sec = sec;
        m_runTime.tv_usec = usec;
    }

    int saveCount() const {
        return m_SaveCount;
    }
    const string& lastOwner() const {
        return m_LastOwner;
    }

private:
    int m_SaveCount = 0;
    string m_LastOwner;
};

Timeval at(long sec, long usec) {
    Timeval t;
    t.tv_sec = sec;
    t.tv_usec = usec;
    return t;
}

} // namespace

TEST(RaceSkillSlotTest, ADefaultSlotHasNoSkillDelayOrCastingTime) {
    RecordingSkillSlot slot;

    EXPECT_EQ(0, slot.getSkillType());
    EXPECT_EQ(0u, slot.getInterval());
    EXPECT_EQ(0u, slot.getCastingTime());
}

TEST(RaceSkillSlotTest, TheConstructorKeepsWhatItIsGiven) {
    RecordingSkillSlot slot(SKILL_DOUBLE_IMPACT, 25, 7);

    EXPECT_EQ(SKILL_DOUBLE_IMPACT, slot.getSkillType());
    EXPECT_EQ(25u, slot.getInterval());
    EXPECT_EQ(7u, slot.getCastingTime());
}

TEST(RaceSkillSlotTest, RemainingTurnsAreTenthsOfASecondUntilTheRunTime) {
    RecordingSkillSlot slot;

    slot.placeRunTime(100, 500000);
    EXPECT_EQ(5u, slot.getRemainTurn(at(100, 0)));

    slot.placeRunTime(102, 0);
    EXPECT_EQ(20u, slot.getRemainTurn(at(100, 0)));
}

TEST(RaceSkillSlotTest, ARunTimeAlreadyPastReportsNoRemainingTurns) {
    // Turn_t is unsigned and GCSkillInfo puts this number on the wire as the
    // skill's remaining casting time, so a slot that came off cooldown
    // reports zero rather than the count near 2^32 the subtraction wraps to.
    RecordingSkillSlot slot;

    // Whole seconds past, and a fraction of a second past.
    slot.placeRunTime(99, 0);
    EXPECT_EQ(0u, slot.getRemainTurn(at(100, 0)));

    slot.placeRunTime(99, 900000);
    EXPECT_EQ(0u, slot.getRemainTurn(at(100, 0)));

    // The run time itself is no longer remaining.
    slot.placeRunTime(100, 0);
    EXPECT_EQ(0u, slot.getRemainTurn(at(100, 0)));
}

TEST(RaceSkillSlotTest, SettingTheRunTimePutsTheDelayAhead) {
    RecordingSkillSlot slot;
    slot.setInterval(30);
    slot.setRunTime();

    Timeval now;
    getCurrentTime(now);

    // The slot is unusable for about the interval and no longer.
    Turn_t remain = slot.getRemainTurn(now);
    EXPECT_GT(remain, 27u);
    EXPECT_LE(remain, 30u);
}

TEST(RaceSkillSlotTest, ACastThatChangesTheDelaySavesItUnderTheOwnerName) {
    RecordingSkillSlot slot(SKILL_DOUBLE_IMPACT, 30, 0);
    slot.setName("Reiot");

    slot.setRunTime(18);

    EXPECT_EQ(18u, slot.getInterval());
    EXPECT_EQ(1, slot.saveCount());
    EXPECT_EQ("Reiot", slot.lastOwner());
}

TEST(RaceSkillSlotTest, ACastThatKeepsTheDelaySavesNothing) {
    RecordingSkillSlot slot(SKILL_DOUBLE_IMPACT, 30, 0);
    slot.setName("Reiot");

    slot.setRunTime(30);

    EXPECT_EQ(0, slot.saveCount());
}

TEST(RaceSkillSlotTest, TheSkillTypesWithNoStoredDelayAreNotSaved) {
    // These are cast from the delay the caller computes, not from a row.
    const SkillType_t unsaved[] = {SKILL_ATTACK_MELEE,    SKILL_ATTACK_ARMS,      SKILL_SELF,       SKILL_TILE,
                                   SKILL_OBJECT,          SKILL_BLOOD_DRAIN,      SKILL_UN_BURROW,  SKILL_UN_TRANSFORM,
                                   SKILL_UN_INVISIBILITY, SKILL_THROW_HOLY_WATER, SKILL_EAT_CORPSE, SKILL_HOWL};

    for (SkillType_t skillType : unsaved) {
        RecordingSkillSlot slot(skillType, 30, 0);
        slot.setName("Reiot");

        slot.setRunTime(18);

        EXPECT_EQ(18u, slot.getInterval()) << "skill type " << (int)skillType;
        EXPECT_EQ(0, slot.saveCount()) << "skill type " << (int)skillType;
    }
}
