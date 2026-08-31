// Pins the Outcome<Events, Rejection> result type (docs/RESTRUCTURING.md
// task 3.1): construction through the named factories, accessors, the
// throw-on-wrong-side contract, and value semantics.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Exception.h"
#include "Outcome.h"

namespace {

typedef std::vector<int> Events;
typedef Outcome<Events, std::string> TestOutcome;

Events makeEvents() {
    Events events;
    events.push_back(7);
    events.push_back(11);
    return events;
}

TEST(OutcomeTest, OkCarriesEvents) {
    TestOutcome outcome = TestOutcome::Ok(makeEvents());

    EXPECT_TRUE(outcome.isOk());
    EXPECT_FALSE(outcome.isRejected());
    ASSERT_EQ(2u, outcome.events().size());
    EXPECT_EQ(7, outcome.events()[0]);
    EXPECT_EQ(11, outcome.events()[1]);
}

TEST(OutcomeTest, RejectedCarriesReason) {
    TestOutcome outcome = TestOutcome::Rejected("out of range");

    EXPECT_FALSE(outcome.isOk());
    EXPECT_TRUE(outcome.isRejected());
    EXPECT_EQ("out of range", outcome.rejection());
}

TEST(OutcomeTest, WrongSideAccessThrows) {
    TestOutcome ok = TestOutcome::Ok(makeEvents());
    TestOutcome rejected = TestOutcome::Rejected("no");

    EXPECT_THROW(ok.rejection(), Exception);
    EXPECT_THROW(rejected.events(), Exception);
}

TEST(OutcomeTest, CopiesAreIndependent) {
    TestOutcome original = TestOutcome::Ok(makeEvents());
    TestOutcome copy = original;

    EXPECT_TRUE(copy.isOk());
    EXPECT_EQ(original.events(), copy.events());

    original = TestOutcome::Rejected("replaced");
    EXPECT_TRUE(original.isRejected());
    EXPECT_TRUE(copy.isOk());
    ASSERT_EQ(2u, copy.events().size());
}

TEST(OutcomeTest, AssignmentSwitchesSides) {
    TestOutcome outcome = TestOutcome::Ok(makeEvents());
    outcome = TestOutcome::Rejected("busy");

    EXPECT_TRUE(outcome.isRejected());
    EXPECT_EQ("busy", outcome.rejection());

    outcome = TestOutcome::Ok(Events());
    EXPECT_TRUE(outcome.isOk());
    EXPECT_TRUE(outcome.events().empty());
}

TEST(OutcomeTest, WorksWithEnumRejection) {
    enum RejectReason { REJECT_NONE, REJECT_OUT_OF_RANGE };
    typedef Outcome<int, RejectReason> IntOutcome;

    IntOutcome outcome = IntOutcome::Rejected(REJECT_OUT_OF_RANGE);
    EXPECT_TRUE(outcome.isRejected());
    EXPECT_EQ(REJECT_OUT_OF_RANGE, outcome.rejection());

    IntOutcome ok = IntOutcome::Ok(42);
    EXPECT_EQ(42, ok.events());
}

} // namespace
