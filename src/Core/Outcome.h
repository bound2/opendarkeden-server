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

#include "Exception.h"

//--------------------------------------------------------------------------------
//
// Outcome<Events, Rejection>
//
// A tagged either-type, C++11, usable by value. Both sides are stored as
// plain members, so Events and Rejection must be default-constructible and
// copyable — true for the intended uses (an event list, an enum or string
// rejection reason). Construct through the named factories only:
//
//     Outcome<AttackEvents, RejectReason> attack(...) {
//         if (outOfRange) return Outcome<AttackEvents, RejectReason>::Rejected(REJECT_OUT_OF_RANGE);
//         ...
//         return Outcome<AttackEvents, RejectReason>::Ok(events);
//     }
//
//     if (outcome.isRejected()) { sendFail(outcome.rejection()); return; }
//     apply(outcome.events());
//
//--------------------------------------------------------------------------------
template <typename Events, typename Rejection> class Outcome {
public:
    static Outcome Ok(const Events& events) {
        Outcome outcome;
        outcome.m_bOk = true;
        outcome.m_Events = events;
        return outcome;
    }

    static Outcome Rejected(const Rejection& reason) {
        Outcome outcome;
        outcome.m_bOk = false;
        outcome.m_Rejection = reason;
        return outcome;
    }

    bool isOk() const {
        return m_bOk;
    }
    bool isRejected() const {
        return !m_bOk;
    }

    // Accessing the side that is not there is a caller bug.
    const Events& events() const {
        if (!m_bOk)
            throw Exception("Outcome::events() on a rejected outcome");
        return m_Events;
    }

    const Rejection& rejection() const {
        if (m_bOk)
            throw Exception("Outcome::rejection() on an ok outcome");
        return m_Rejection;
    }

private:
    Outcome() : m_bOk(false), m_Events(), m_Rejection() {}

    bool m_bOk;
    Events m_Events;
    Rejection m_Rejection;
};

#endif
