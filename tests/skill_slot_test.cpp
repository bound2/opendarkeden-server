// SkillSlot is the slayer's learned-skill slot. It carries the same
// run-time arithmetic as the vampire and ousters slots (RaceSkillSlot), and
// the number it computes leaves the server as a casting time in GCSkillInfo,
// so the rule it follows for a cooldown that has already elapsed is pinned
// here as well.

#include <gtest/gtest.h>

#include "repository/SkillSaveRepository.h"
#include "skill/SkillSlot.h"

namespace {

// SkillSlot::save() goes to the process-wide repository; the tests stand one
// in that records instead of writing rows.
class RecordingSkillSaveRepository : public SkillSaveRepository {
public:
    std::vector<SlayerSkillRow> loadSlayerSkills(const std::string&) override {
        return {};
    }
    std::vector<VampireSkillRow> loadVampireSkills(const std::string&) override {
        return {};
    }
    std::vector<OustersSkillRow> loadOustersSkills(const std::string&) override {
        return {};
    }

    void insertSlayerSkill(const std::string&, const SlayerSkillRecord&) override {}
    void insertVampireSkill(const std::string&, const VampireSkillRecord&) override {}
    void insertOustersSkill(const std::string&, const OustersSkillRecord&) override {}

    void updateSlayerSkill(const std::string& ownerName, SkillType_t, ExpLevel_t, Exp_t, Turn_t delay) override {
        ++m_UpdateCount;
        m_LastOwner = ownerName;
        m_LastDelay = delay;
    }
    void updateVampireSkill(const std::string&, SkillType_t, Turn_t) override {}
    void updateOustersSkill(const std::string&, SkillType_t, ExpLevel_t, Turn_t) override {}

    void deleteOustersSkill(const std::string&, SkillType_t) override {}

    int updateCount() const {
        return m_UpdateCount;
    }
    const std::string& lastOwner() const {
        return m_LastOwner;
    }
    Turn_t lastDelay() const {
        return m_LastDelay;
    }
    void forget() {
        m_UpdateCount = 0;
        m_LastOwner.clear();
        m_LastDelay = 0;
    }

private:
    int m_UpdateCount = 0;
    std::string m_LastOwner;
    Turn_t m_LastDelay = 0;
};

RecordingSkillSaveRepository g_Repository;

// The run time is normally read from the clock; the tests place it.
class TestSkillSlot : public SkillSlot {
public:
    TestSkillSlot() : SkillSlot() {}
    TestSkillSlot(SkillType_t skillType, DWORD exp, ulong interval) : SkillSlot(skillType, exp, interval) {}

    void placeRunTime(long sec, long usec) {
        m_runTime.tv_sec = sec;
        m_runTime.tv_usec = usec;
    }
};

Timeval at(long sec, long usec) {
    Timeval t;
    t.tv_sec = sec;
    t.tv_usec = usec;
    return t;
}

} // namespace

SkillSaveRepository& defaultSkillSaveRepository() {
    return g_Repository;
}

TEST(SkillSlotTest, RemainingTurnsAreTenthsOfASecondUntilTheRunTime) {
    TestSkillSlot slot;

    slot.placeRunTime(100, 500000);
    EXPECT_EQ(5u, slot.getRemainTurn(at(100, 0)));

    slot.placeRunTime(102, 0);
    EXPECT_EQ(20u, slot.getRemainTurn(at(100, 0)));
}

TEST(SkillSlotTest, ARunTimeAlreadyPastReportsNoRemainingTurns) {
    // Turn_t is unsigned and GCSkillInfo puts this number on the wire as the
    // skill's remaining casting time, so a slot that came off cooldown
    // reports zero rather than the count near 2^32 the subtraction wraps to.
    TestSkillSlot slot;

    // Whole seconds past, and a fraction of a second past.
    slot.placeRunTime(99, 0);
    EXPECT_EQ(0u, slot.getRemainTurn(at(100, 0)));

    slot.placeRunTime(99, 900000);
    EXPECT_EQ(0u, slot.getRemainTurn(at(100, 0)));

    // The run time itself is no longer remaining.
    slot.placeRunTime(100, 0);
    EXPECT_EQ(0u, slot.getRemainTurn(at(100, 0)));
}

TEST(SkillSlotTest, ACastThatChangesTheDelaySavesItUnderTheOwnerName) {
    g_Repository.forget();

    TestSkillSlot slot(SKILL_DOUBLE_IMPACT, 0, 30);
    slot.setName("Reiot");

    slot.setRunTime(18);

    EXPECT_EQ(18u, slot.getInterval());
    EXPECT_EQ(1, g_Repository.updateCount());
    EXPECT_EQ("Reiot", g_Repository.lastOwner());
    EXPECT_EQ(18u, g_Repository.lastDelay());
}

TEST(SkillSlotTest, ACastAskedNotToSaveKeepsTheDelayOutOfTheTable) {
    g_Repository.forget();

    TestSkillSlot slot(SKILL_DOUBLE_IMPACT, 0, 30);
    slot.setName("Reiot");

    slot.setRunTime(18, false);

    // The interval is only rewritten on the saving path.
    EXPECT_EQ(30u, slot.getInterval());
    EXPECT_EQ(0, g_Repository.updateCount());
}
