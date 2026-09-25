//////////////////////////////////////////////////////////////////////
//
// Filename    : database_manager_test.cpp
// Description : The database manager's per-thread connection tables
//               (src/server/database/DatabaseManager.h), registered and
//               looked up from many threads at once.
//
// Every worker registers its own connection when it starts, while the
// workers already running look theirs up for every statement. Nothing
// here connects: a default-constructed Connection is an initialised,
// unconnected MySQL handle, which is all the tables hold.
//
//////////////////////////////////////////////////////////////////////

#include <atomic>
#include <latch>
#include <memory>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "Connection.h"
#include "DatabaseManager.h"
#include "Exception.h"
#include "Thread.h"

// Threads start together, so registrations land while other threads are
// already inside lookups. Each keeps running until every one has finished,
// so no thread id is reused by a later thread while the tables still hold it.
TEST(DatabaseManagerTables, EveryThreadFindsItsOwnConnectionWhileOthersRegister) {
    const int kThreads = 16;
    const int kLookups = 2000;

    // The manager frees the connections registered with addConnection; the
    // distribution connections stay the caller's.
    std::vector<Connection*> own;
    std::vector<std::unique_ptr<Connection>> dist;
    for (int i = 0; i < kThreads; i++) {
        own.push_back(new Connection());
        dist.push_back(std::make_unique<Connection>());
    }

    DatabaseManager manager;
    std::atomic<int> mismatches{0};
    std::atomic<int> registrationFailures{0};
    std::latch start(kThreads);
    std::latch finished(kThreads);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; i++) {
        threads.emplace_back([&, i]() {
            start.arrive_and_wait();

            // Before its own registration a thread reaches the default
            // connection, which is null in a manager init() never ran on.
            if (manager.getConnection("DARKEDEN") != nullptr || manager.getDistConnection("PLAYER_DB") != nullptr)
                mismatches++;

            try {
                const int tid = (int)(long)Thread::self();
                manager.addConnection(tid, own[i]);
                manager.addDistConnection(tid, dist[i].get());
            } catch (DuplicatedException&) {
                registrationFailures++;
            }

            for (int n = 0; n < kLookups; n++) {
                if (manager.getConnection("DARKEDEN") != own[i])
                    mismatches++;
                if (manager.getDistConnection("PLAYER_DB") != dist[i].get())
                    mismatches++;
            }

            finished.arrive_and_wait();
        });
    }
    for (std::thread& thread : threads)
        thread.join();

    EXPECT_EQ(0, registrationFailures.load());
    EXPECT_EQ(0, mismatches.load());
}

// A second registration under one id is refused and the first one stays.
TEST(DatabaseManagerTables, ASecondRegistrationUnderOneIdIsRefused) {
    DatabaseManager manager;
    Connection* pFirst = new Connection();
    auto second = std::make_unique<Connection>();
    const int tid = (int)(long)Thread::self();

    manager.addConnection(tid, pFirst);
    EXPECT_THROW(manager.addConnection(tid, second.get()), DuplicatedException);
    EXPECT_EQ(pFirst, manager.getConnection("DARKEDEN"));

    auto firstDist = std::make_unique<Connection>();
    manager.addDistConnection(tid, firstDist.get());
    EXPECT_THROW(manager.addDistConnection(tid, second.get()), DuplicatedException);
    EXPECT_EQ(firstDist.get(), manager.getDistConnection("PLAYER_DB"));
}
