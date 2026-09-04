//--------------------------------------------------------------------------------
//
// Filename   : Outcome.h
//
// The result type for gameplay mutations (docs/RESTRUCTURING.md task 3.1):
// domain code returns Ok(events) or Rejected(reason) instead of using
// exceptions for control flow. Exceptions stay reserved for programming and
// configuration errors, which is why accessing the wrong side throws.
//
//--------------------------------------------------------------------------------

#ifndef __OUTCOME_H__
#define __OUTCOME_H__

#include <stdexcept>
#include <utility>
#include <variant>

// Thrown on wrong-side access. Deliberately derives from std::logic_error,
// not this codebase's Throwable: the legacy __END_CATCH_NO_RETHROW macros
// swallow every Throwable, which would silently defeat this contract.
class OutcomeContractViolation : public std::logic_error {
public:
    explicit OutcomeContractViolation(const char* what) : std::logic_error(what) {}
};

// A tagged either-type backed by std::variant. Only the active side is stored,
// so payloads need not be default-constructible. There is no public default
// constructor: an Outcome always comes from one of the named factories.
//
//     [[nodiscard]] Outcome<AttackEvents, RejectReason> attack(...);
//
// Accessors on lvalues return references into the Outcome. Accessors on
// rvalues return by value and move the payload out, avoiding references into
// temporary Outcomes. Use Outcome<void, Rejection> when success has no payload.
template <typename Events, typename Rejection> class [[nodiscard]] Outcome {
public:
    // By value plus move: an rvalue payload is moved once; an lvalue pays the
    // one copy needed to put an independent value into the Outcome.
    static Outcome Ok(Events events) {
        return Outcome(std::in_place_index<0>, std::move(events));
    }

    static Outcome Rejected(Rejection reason) {
        return Outcome(std::in_place_index<1>, std::move(reason));
    }

    bool isOk() const {
        return m_Value.index() == 0;
    }

    bool isRejected() const {
        return !isOk();
    }

    const Events& events() const& {
        requireOk();
        return std::get<0>(m_Value);
    }

    Events events() && {
        requireOk();
        return std::get<0>(std::move(m_Value));
    }

    const Rejection& rejection() const& {
        requireRejected();
        return std::get<1>(m_Value);
    }

    Rejection rejection() && {
        requireRejected();
        return std::get<1>(std::move(m_Value));
    }

private:
    Outcome(std::in_place_index_t<0> index, Events&& events) : m_Value(index, std::move(events)) {}
    Outcome(std::in_place_index_t<1> index, Rejection&& reason) : m_Value(index, std::move(reason)) {}

    void requireOk() const {
        if (!isOk())
            throw OutcomeContractViolation("Outcome::events() on a rejected outcome");
    }

    void requireRejected() const {
        if (isOk())
            throw OutcomeContractViolation("Outcome::rejection() on an ok outcome");
    }

    std::variant<Events, Rejection> m_Value;
};

// The no-payload form: the mutation succeeded, or was rejected with a reason.
template <typename Rejection> class [[nodiscard]] Outcome<void, Rejection> {
public:
    static Outcome Ok() {
        return Outcome();
    }

    static Outcome Rejected(Rejection reason) {
        return Outcome(std::move(reason));
    }

    bool isOk() const {
        return m_Value.index() == 0;
    }

    bool isRejected() const {
        return !isOk();
    }

    const Rejection& rejection() const& {
        requireRejected();
        return std::get<1>(m_Value);
    }

    Rejection rejection() && {
        requireRejected();
        return std::get<1>(std::move(m_Value));
    }

private:
    Outcome() : m_Value(std::in_place_index<0>) {}
    explicit Outcome(Rejection&& reason) : m_Value(std::in_place_index<1>, std::move(reason)) {}

    void requireRejected() const {
        if (isOk())
            throw OutcomeContractViolation("Outcome::rejection() on an ok outcome");
    }

    std::variant<std::monostate, Rejection> m_Value;
};

#endif
