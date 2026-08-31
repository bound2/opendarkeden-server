//--------------------------------------------------------------------------------
//
// Filename   : Outcome.h
//
// The result type for gameplay mutations (docs/RESTRUCTURING.md task 3.1):
// domain code returns Ok(events) or Rejected(reason) instead of using
// exceptions for control flow. Exceptions stay reserved for programming and
// configuration errors — which is why accessing the wrong side of an Outcome
// throws: that is a caller bug, not a domain outcome.
//
//--------------------------------------------------------------------------------

#ifndef __OUTCOME_H__
#define __OUTCOME_H__

#include <stdexcept>
#include <utility>

#include <type_traits>

// Mark a function whose Outcome must not be dropped: a discarded result
// silently turns a rejected mutation into a no-op, which is exactly the
// failure mode this type exists to prevent. Annotate every domain function
// that returns an Outcome:
//
//     DE_MUST_USE Outcome<AttackEvents, RejectReason> attack(...);
//
#if defined(__GNUC__) || defined(__clang__)
#define DE_MUST_USE __attribute__((warn_unused_result))
#else
#define DE_MUST_USE
#endif

// Thrown on wrong-side access. Deliberately derives from std::logic_error,
// NOT from this codebase's Throwable: the legacy __END_CATCH_NO_RETHROW
// macros swallow every Throwable (347 sites in the gameserver — the very
// code being migrated to Outcome), which would silently defeat the
// contract. A std::logic_error escapes them all and surfaces at the
// outermost handler, loudly.
class OutcomeContractViolation : public std::logic_error {
public:
    explicit OutcomeContractViolation(const char* what) : std::logic_error(what) {}
};

//--------------------------------------------------------------------------------
//
// Outcome<Events, Rejection>
//
// A tagged either-type, C++11, usable by value. Both sides are stored as
// plain members, so Events and Rejection must be default-constructible and
// copyable (enforced below) — true for the intended uses (an event list, an
// enum or string rejection reason). There is no default constructor: an
// Outcome always comes from one of the named factories, so it cannot be used
// as a std::map mapped type via operator[], in vector::resize, or as a class
// member without an initializer.
//
//     DE_MUST_USE Outcome<AttackEvents, RejectReason> attack(...);
//
//     Outcome<AttackEvents, RejectReason> attack(...) {
//         if (outOfRange) return Outcome<AttackEvents, RejectReason>::Rejected(REJECT_OUT_OF_RANGE);
//         ...
//         return Outcome<AttackEvents, RejectReason>::Ok(events);  // moves an rvalue in
//     }
//
//     Outcome<AttackEvents, RejectReason> outcome = attack(target);
//     if (outcome.isRejected()) { sendFail(outcome.rejection()); return; }
//     apply(outcome.events());
//
// Accessor lifetime: events()/rejection() on an lvalue return a reference
// into the Outcome — never bind one past the Outcome's own lifetime. Called
// on an rvalue (attack(t).events(), or std::move(outcome).events()) they
// return BY VALUE, moving the payload out, so the temporary-dangling trap of
// `for (const Event& e : attack(t).events())` cannot arise and consumers
// that want ownership get it without a copy.
//
// Mutations with no event payload use the Outcome<void, Rejection>
// specialization below: Ok() takes nothing and there is no events().
// Propagating a rejection outward across differing Events types is written
// out by hand at each layer (C++11, no std::expected):
//
//     if (inner.isRejected()) return Outcome<Outer, RejectReason>::Rejected(inner.rejection());
//
//--------------------------------------------------------------------------------
template <typename Events, typename Rejection> class Outcome {
    static_assert(std::is_default_constructible<Events>::value,
                  "Outcome stores both sides as plain members: Events must be default-constructible");
    static_assert(std::is_copy_constructible<Events>::value, "Events must be copyable");
    static_assert(std::is_default_constructible<Rejection>::value,
                  "Outcome stores both sides as plain members: Rejection must be default-constructible");
    static_assert(std::is_copy_constructible<Rejection>::value, "Rejection must be copyable");

public:
    // By value + move: Ok(makeEvents()) moves the temporary in instead of
    // deep-copying it; an lvalue argument costs the one copy it must.
    DE_MUST_USE static Outcome Ok(Events events) {
        Outcome outcome;
        outcome.m_bOk = true;
        outcome.m_Events = std::move(events);
        return outcome;
    }

    DE_MUST_USE static Outcome Rejected(Rejection reason) {
        Outcome outcome;
        outcome.m_bOk = false;
        outcome.m_Rejection = std::move(reason);
        return outcome;
    }

    bool isOk() const {
        return m_bOk;
    }
    bool isRejected() const {
        return !m_bOk;
    }

    // Accessing the side that is not there is a caller bug (see
    // OutcomeContractViolation above for why this is not a Throwable).
    const Events& events() const& {
        requireOk();
        return m_Events;
    }
    // rvalue overload: moves the payload out, and makes calling events() on
    // a temporary safe (a value, not a reference into a dead Outcome).
    Events events() && {
        requireOk();
        return std::move(m_Events);
    }

    const Rejection& rejection() const& {
        requireRejected();
        return m_Rejection;
    }
    Rejection rejection() && {
        requireRejected();
        return std::move(m_Rejection);
    }

private:
    Outcome() : m_bOk(false), m_Events(), m_Rejection() {}

    void requireOk() const {
        if (!m_bOk)
            throw OutcomeContractViolation("Outcome::events() on a rejected outcome");
    }
    void requireRejected() const {
        if (m_bOk)
            throw OutcomeContractViolation("Outcome::rejection() on an ok outcome");
    }

    bool m_bOk;
    Events m_Events;
    Rejection m_Rejection;
};

// The no-payload form: the mutation succeeded, or was rejected with a
// reason. The single most common gameplay outcome.
template <typename Rejection> class Outcome<void, Rejection> {
    static_assert(std::is_default_constructible<Rejection>::value,
                  "Outcome stores the rejection as a plain member: Rejection must be default-constructible");
    static_assert(std::is_copy_constructible<Rejection>::value, "Rejection must be copyable");

public:
    DE_MUST_USE static Outcome Ok() {
        Outcome outcome;
        outcome.m_bOk = true;
        return outcome;
    }

    DE_MUST_USE static Outcome Rejected(Rejection reason) {
        Outcome outcome;
        outcome.m_bOk = false;
        outcome.m_Rejection = std::move(reason);
        return outcome;
    }

    bool isOk() const {
        return m_bOk;
    }
    bool isRejected() const {
        return !m_bOk;
    }

    const Rejection& rejection() const& {
        requireRejected();
        return m_Rejection;
    }
    Rejection rejection() && {
        requireRejected();
        return std::move(m_Rejection);
    }

private:
    Outcome() : m_bOk(false), m_Rejection() {}

    void requireRejected() const {
        if (m_bOk)
            throw OutcomeContractViolation("Outcome::rejection() on an ok outcome");
    }

    bool m_bOk;
    Rejection m_Rejection;
};

#endif
