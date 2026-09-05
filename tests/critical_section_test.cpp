// critical_section_test.cpp — the RAII contract of
// __ENTER_CRITICAL_SECTION / __LEAVE_CRITICAL_SECTION (src/Core/Exception.h).
//
// The macros used to expand to a bare mutex.lock() plus a
// "catch (Throwable&) { mutex.unlock(); throw; }" tail, so the lock leaked on
// any exception that was not a Throwable and on every return that did not
// unlock by hand. They now bind a scoped CriticalSection guard, and these
// tests pin the exits that guard has to cover.
//
// The lockable here only counts calls, so a leak shows up as an imbalance
// rather than as a hung test. Links only de-kernel (Exception.h is a kernel
// header) plus gtest.

#include <stdexcept>

#include <gtest/gtest.h>

#include "Exception.h"

namespace {

// Minimal BasicLockable: lock()/unlock() are all the guard may use.
class CountingLockable {
public:
    void lock() {
        ++m_Locks;
        m_Held = true;
    }

    void unlock() {
        ++m_Unlocks;
        m_Held = false;
    }

    int locks() const {
        return m_Locks;
    }
    int unlocks() const {
        return m_Unlocks;
    }
    bool held() const {
        return m_Held;
    }
    bool balanced() const {
        return m_Locks == m_Unlocks;
    }

private:
    int m_Locks = 0;
    int m_Unlocks = 0;
    bool m_Held = false;
};

// Lockable whose lock() fails, to pin that a failed acquire unlocks nothing.
class ThrowingLockable {
public:
    void lock() {
        ++m_Locks;
        throw Error("ThrowingLockable::lock");
    }

    void unlock() {
        ++m_Unlocks;
    }

    int locks() const {
        return m_Locks;
    }
    int unlocks() const {
        return m_Unlocks;
    }

private:
    int m_Locks = 0;
    int m_Unlocks = 0;
};

// Helpers that exercise the macros exactly as production code writes them.

void runToEnd(CountingLockable& m, bool& sawInside) {
    __ENTER_CRITICAL_SECTION(m)

    sawInside = m.held();

    __LEAVE_CRITICAL_SECTION(m)
}

int returnFromInside(CountingLockable& m) {
    __ENTER_CRITICAL_SECTION(m)

    return 7;

    __LEAVE_CRITICAL_SECTION(m)
}

void throwThrowableFromInside(CountingLockable& m) {
    __ENTER_CRITICAL_SECTION(m)

    throw Error("from inside a critical section");

    __LEAVE_CRITICAL_SECTION(m)
}

void throwStdExceptionFromInside(CountingLockable& m) {
    __ENTER_CRITICAL_SECTION(m)

    throw std::out_of_range("not a Throwable");

    __LEAVE_CRITICAL_SECTION(m)
}

void throwNonClassTypeFromInside(CountingLockable& m) {
    __ENTER_CRITICAL_SECTION(m)

    throw 42;

    __LEAVE_CRITICAL_SECTION(m)
}

// A section inside a loop, left with `continue` — RegenZoneManager.cpp does
// exactly this.
int continueOutOfSection(CountingLockable& m) {
    int reached = 0;

    for (int i = 0; i < 3; ++i) {
        __ENTER_CRITICAL_SECTION(m)

        if (i < 2)
            continue;

        ++reached;

        __LEAVE_CRITICAL_SECTION(m)
    }

    return reached;
}

// A section left with `goto`, as MasterLairManager::enterCreature does.
bool gotoOutOfSection(CountingLockable& m) {
    __ENTER_CRITICAL_SECTION(m)

    goto done;

    __LEAVE_CRITICAL_SECTION(m)

done:
    return true;
}

// __CRITICAL_SECTION_LOCK: release inside the block, then return. Party.cpp's
// shareAttrExp family does this so the expensive share work runs unlocked.
int releaseEarlyThenReturn(CountingLockable& m, bool& heldAfterRelease) {
    __ENTER_CRITICAL_SECTION(m)

    __CRITICAL_SECTION_LOCK.unlock();

    heldAfterRelease = m.held();

    return 3;

    __LEAVE_CRITICAL_SECTION(m)
}

// Release, do work, retake — the state has to be consistent at scope exit.
void releaseAndRetake(CountingLockable& m, bool& heldInGap, bool& heldAtEnd) {
    __ENTER_CRITICAL_SECTION(m)

    __CRITICAL_SECTION_LOCK.unlock();
    heldInGap = m.held();

    __CRITICAL_SECTION_LOCK.lock();
    heldAtEnd = m.held();

    __LEAVE_CRITICAL_SECTION(m)
}

void nestedSections(CountingLockable& outer, CountingLockable& inner, bool& bothHeld) {
    __ENTER_CRITICAL_SECTION(outer)
    __ENTER_CRITICAL_SECTION(inner)

    bothHeld = outer.held() && inner.held();

    __LEAVE_CRITICAL_SECTION(inner)
    __LEAVE_CRITICAL_SECTION(outer)
}

// The macro argument must be evaluated once, at __ENTER: the guard keeps the
// address. The old macro re-evaluated it at __LEAVE and in its catch.
int g_ResolveCalls = 0;

CountingLockable& resolve(CountingLockable& m) {
    ++g_ResolveCalls;
    return m;
}

} // namespace

TEST(CriticalSection, ReleasesOnNormalExit) {
    CountingLockable m;
    bool sawInside = false;

    runToEnd(m, sawInside);

    EXPECT_TRUE(sawInside);
    EXPECT_EQ(1, m.locks());
    EXPECT_EQ(1, m.unlocks());
    EXPECT_FALSE(m.held());
}

TEST(CriticalSection, ReleasesOnReturn) {
    CountingLockable m;

    EXPECT_EQ(7, returnFromInside(m));

    EXPECT_EQ(1, m.locks());
    EXPECT_EQ(1, m.unlocks());
    EXPECT_FALSE(m.held());
}

TEST(CriticalSection, ReleasesOnThrowable) {
    CountingLockable m;

    EXPECT_THROW(throwThrowableFromInside(m), Error);

    EXPECT_EQ(1, m.locks());
    EXPECT_EQ(1, m.unlocks());
    EXPECT_FALSE(m.held());
}

// The regression the old macro could not handle: its catch clause named
// Throwable, so a std::exception walked past it with the mutex still held.
TEST(CriticalSection, ReleasesOnStdException) {
    CountingLockable m;

    EXPECT_THROW(throwStdExceptionFromInside(m), std::out_of_range);

    EXPECT_EQ(1, m.locks());
    EXPECT_EQ(1, m.unlocks());
    EXPECT_FALSE(m.held());
}

TEST(CriticalSection, ReleasesOnNonClassException) {
    CountingLockable m;

    EXPECT_THROW(throwNonClassTypeFromInside(m), int);

    EXPECT_EQ(1, m.locks());
    EXPECT_EQ(1, m.unlocks());
    EXPECT_FALSE(m.held());
}

TEST(CriticalSection, ReleasesOnContinue) {
    CountingLockable m;

    EXPECT_EQ(1, continueOutOfSection(m));

    EXPECT_EQ(3, m.locks());
    EXPECT_EQ(3, m.unlocks());
    EXPECT_FALSE(m.held());
}

TEST(CriticalSection, ReleasesOnGoto) {
    CountingLockable m;

    EXPECT_TRUE(gotoOutOfSection(m));

    EXPECT_EQ(1, m.locks());
    EXPECT_EQ(1, m.unlocks());
    EXPECT_FALSE(m.held());
}

TEST(CriticalSection, EarlyReleaseThroughGuardDoesNotUnlockTwice) {
    CountingLockable m;
    bool heldAfterRelease = true;

    EXPECT_EQ(3, releaseEarlyThenReturn(m, heldAfterRelease));

    EXPECT_FALSE(heldAfterRelease);
    EXPECT_EQ(1, m.locks());
    EXPECT_EQ(1, m.unlocks());
    EXPECT_TRUE(m.balanced());
}

TEST(CriticalSection, ReleaseAndRetakeStaysBalanced) {
    CountingLockable m;
    bool heldInGap = true;
    bool heldAtEnd = false;

    releaseAndRetake(m, heldInGap, heldAtEnd);

    EXPECT_FALSE(heldInGap);
    EXPECT_TRUE(heldAtEnd);
    EXPECT_EQ(2, m.locks());
    EXPECT_EQ(2, m.unlocks());
    EXPECT_FALSE(m.held());
}

TEST(CriticalSection, NestedSectionsBothRelease) {
    CountingLockable outer;
    CountingLockable inner;
    bool bothHeld = false;

    nestedSections(outer, inner, bothHeld);

    EXPECT_TRUE(bothHeld);
    EXPECT_EQ(1, outer.unlocks());
    EXPECT_EQ(1, inner.unlocks());
    EXPECT_FALSE(outer.held());
    EXPECT_FALSE(inner.held());
}

TEST(CriticalSection, ArgumentIsEvaluatedOnce) {
    CountingLockable m;
    g_ResolveCalls = 0;

    __ENTER_CRITICAL_SECTION(resolve(m))
    __LEAVE_CRITICAL_SECTION(resolve(m))

    EXPECT_EQ(1, g_ResolveCalls);
    EXPECT_EQ(1, m.locks());
    EXPECT_EQ(1, m.unlocks());
}

// Mutex::lock() throws Error on self-deadlock. The guard acquires before it
// owns anything, so a failed acquire must not produce an unlock.
TEST(CriticalSection, FailedAcquireUnlocksNothing) {
    ThrowingLockable m;

    EXPECT_THROW({__ENTER_CRITICAL_SECTION(m) __LEAVE_CRITICAL_SECTION(m)}, Error);

    EXPECT_EQ(1, m.locks());
    EXPECT_EQ(0, m.unlocks());
}

// The guard is usable on its own, not only through the macros.
TEST(CriticalSection, GuardTracksOwnership) {
    CountingLockable m;

    {
        CriticalSection guard{m};
        EXPECT_TRUE(guard.ownsLock());

        guard.unlock();
        EXPECT_FALSE(guard.ownsLock());

        // Redundant releases are absorbed rather than passed to the lockable.
        guard.unlock();
        EXPECT_EQ(1, m.unlocks());

        guard.lock();
        EXPECT_TRUE(guard.ownsLock());
        guard.lock();
        EXPECT_EQ(2, m.locks());
    }

    EXPECT_EQ(2, m.locks());
    EXPECT_EQ(2, m.unlocks());
}
