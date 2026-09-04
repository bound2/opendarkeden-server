// Pins the Outcome<Events, Rejection> result type (docs/RESTRUCTURING.md
// task 3.1): construction through the named factories, accessors, the
// throw-on-wrong-side contract, value/move semantics, rvalue move-out, and
// the void specialization.

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <type_traits>

#include "Exception.h"
#include "Outcome.h"

namespace {

typedef std::vector<int> Events;
typedef Outcome<Events, std::string> TestOutcome;

// The contract-violation type must NOT sit in the legacy Throwable
// hierarchy: __END_CATCH_NO_RETHROW swallows every Throwable, which would
// silently defeat the wrong-side contract in exactly the code Outcome is
// migrating. Pinned at compile time.
static_assert(!std::is_base_of<Throwable, OutcomeContractViolation>::value,
              "OutcomeContractViolation must escape the legacy catch(Throwable&) macros");
static_assert(std::is_base_of<std::logic_error, OutcomeContractViolation>::value,
              "OutcomeContractViolation is a std::logic_error");

Events makeEvents() {
    Events events;
    events.push_back(7);
    events.push_back(11);
    return events;
}

class NonDefaultPayload {
public:
    explicit NonDefaultPayload(int value) : m_Value(value) {}

    int value() const {
        return m_Value;
    }

private:
    int m_Value;
};

static_assert(!std::is_default_constructible<NonDefaultPayload>::value,
              "the variant test payload must not be default-constructible");

class MoveCountedPayload {
public:
    explicit MoveCountedPayload(int value) : m_Value(value) {}
    MoveCountedPayload(const MoveCountedPayload& other) : m_Value(other.m_Value) {
        ++s_Copies;
    }
    MoveCountedPayload(MoveCountedPayload&& other) noexcept : m_Value(other.m_Value) {
        ++s_Moves;
    }
    MoveCountedPayload& operator=(const MoveCountedPayload&) = default;
    MoveCountedPayload& operator=(MoveCountedPayload&&) = default;

    static void resetCounts() {
        s_Copies = 0;
        s_Moves = 0;
    }
    static int copies() {
        return s_Copies;
    }
    static int moves() {
        return s_Moves;
    }

private:
    int m_Value;
    static int s_Copies;
    static int s_Moves;
};

int MoveCountedPayload::s_Copies = 0;
int MoveCountedPayload::s_Moves = 0;

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

    EXPECT_THROW(ok.rejection(), OutcomeContractViolation);
    EXPECT_THROW(rejected.events(), OutcomeContractViolation);
    // and the violation is catchable at the std level, not the Throwable one
    EXPECT_THROW(ok.rejection(), std::logic_error);
}

TEST(OutcomeTest, ConstObjectAccessors) {
    const TestOutcome ok = TestOutcome::Ok(makeEvents());
    const TestOutcome rejected = TestOutcome::Rejected("no");

    ASSERT_EQ(2u, ok.events().size());
    EXPECT_EQ("no", rejected.rejection());
    EXPECT_THROW(ok.rejection(), OutcomeContractViolation);
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

TEST(OutcomeTest, LvalueCopyAssignmentKeepsSource) {
    TestOutcome source = TestOutcome::Ok(makeEvents());
    TestOutcome target = TestOutcome::Rejected("stale");

    target = source; // copy-assign from an lvalue, not a prvalue

    EXPECT_TRUE(target.isOk());
    ASSERT_EQ(2u, target.events().size());
    EXPECT_TRUE(source.isOk());
    ASSERT_EQ(2u, source.events().size());
}

TEST(OutcomeTest, MoveConstruction) {
    TestOutcome source = TestOutcome::Ok(makeEvents());
    TestOutcome moved(std::move(source));

    EXPECT_TRUE(moved.isOk());
    ASSERT_EQ(2u, moved.events().size());
    EXPECT_EQ(7, moved.events()[0]);
}

TEST(OutcomeTest, AssignmentSwitchesSides) {
    TestOutcome outcome = TestOutcome::Ok(makeEvents());
    outcome = TestOutcome::Rejected("busy");

    EXPECT_TRUE(outcome.isRejected());
    EXPECT_EQ("busy", outcome.rejection());
    // the stale side is unreachable after the switch
    EXPECT_THROW(outcome.events(), OutcomeContractViolation);

    outcome = TestOutcome::Ok(Events());
    EXPECT_TRUE(outcome.isOk());
    EXPECT_TRUE(outcome.events().empty());
    EXPECT_THROW(outcome.rejection(), OutcomeContractViolation);
}

TestOutcome makeOk() {
    return TestOutcome::Ok(makeEvents());
}

TEST(OutcomeTest, RvalueAccessorMovesOut) {
    // events() on an rvalue returns BY VALUE (moved out), so binding or
    // iterating a call-result temporary is safe — no dangling reference.
    Events taken = makeOk().events();
    ASSERT_EQ(2u, taken.size());
    EXPECT_EQ(11, taken[1]);

    int sum = 0;
    Events fromTemp = makeOk().events();
    for (Events::const_iterator it = fromTemp.begin(); it != fromTemp.end(); ++it)
        sum += *it;
    EXPECT_EQ(18, sum);

    TestOutcome source = TestOutcome::Ok(makeEvents());
    Events movedOut = std::move(source).events();
    ASSERT_EQ(2u, movedOut.size());
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

TEST(OutcomeTest, VariantStoresNonDefaultConstructiblePayloads) {
    typedef Outcome<NonDefaultPayload, NonDefaultPayload> NonDefaultOutcome;

    NonDefaultOutcome ok = NonDefaultOutcome::Ok(NonDefaultPayload(23));
    EXPECT_TRUE(ok.isOk());
    EXPECT_EQ(23, ok.events().value());

    NonDefaultOutcome rejected = NonDefaultOutcome::Rejected(NonDefaultPayload(47));
    EXPECT_TRUE(rejected.isRejected());
    EXPECT_EQ(47, rejected.rejection().value());
}

TEST(OutcomeTest, VariantSupportsMoveOnlyPayloads) {
    typedef Outcome<std::unique_ptr<int>, std::string> MoveOnlyOutcome;
    static_assert(!std::is_copy_constructible<MoveOnlyOutcome>::value,
                  "Outcome copyability must follow its variant payloads");

    MoveOnlyOutcome outcome = MoveOnlyOutcome::Ok(std::unique_ptr<int>(new int(31)));
    ASSERT_NE(nullptr, outcome.events());
    EXPECT_EQ(31, *outcome.events());

    std::unique_ptr<int> payload = std::move(outcome).events();
    ASSERT_NE(nullptr, payload);
    EXPECT_EQ(31, *payload);
}

TEST(OutcomeTest, FactoriesMoveTemporaryPayloadOnce) {
    typedef Outcome<MoveCountedPayload, std::string> EventOutcome;
    MoveCountedPayload::resetCounts();
    EventOutcome ok = EventOutcome::Ok(MoveCountedPayload(31));
    EXPECT_TRUE(ok.isOk());
    EXPECT_EQ(0, MoveCountedPayload::copies());
    EXPECT_EQ(1, MoveCountedPayload::moves());

    typedef Outcome<std::string, MoveCountedPayload> RejectedOutcome;
    MoveCountedPayload::resetCounts();
    RejectedOutcome rejected = RejectedOutcome::Rejected(MoveCountedPayload(47));
    EXPECT_TRUE(rejected.isRejected());
    EXPECT_EQ(0, MoveCountedPayload::copies());
    EXPECT_EQ(1, MoveCountedPayload::moves());

    typedef Outcome<void, MoveCountedPayload> VoidOutcome;
    MoveCountedPayload::resetCounts();
    VoidOutcome voidRejected = VoidOutcome::Rejected(MoveCountedPayload(59));
    EXPECT_TRUE(voidRejected.isRejected());
    EXPECT_EQ(0, MoveCountedPayload::copies());
    EXPECT_EQ(1, MoveCountedPayload::moves());
}

TEST(OutcomeTest, VoidSpecialization) {
    typedef Outcome<void, std::string> Ack;

    Ack ok = Ack::Ok();
    EXPECT_TRUE(ok.isOk());
    EXPECT_FALSE(ok.isRejected());
    EXPECT_THROW(ok.rejection(), OutcomeContractViolation);

    Ack rejected = Ack::Rejected("not yours");
    EXPECT_TRUE(rejected.isRejected());
    EXPECT_EQ("not yours", rejected.rejection());

    ok = rejected;
    EXPECT_TRUE(ok.isRejected());
}

} // namespace
