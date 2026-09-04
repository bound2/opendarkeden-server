#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <stdexcept>

#include <condition_variable>
#include <gtest/gtest.h>

#include "CooperativeThread.h"

using namespace std::chrono_literals;

TEST(CooperativeThreadTest, DestructorRequestsStopAndJoins) {
    std::promise<void> started;
    std::future<void> ready = started.get_future();
    std::atomic<bool> observedStop = false;
    std::mutex mutex;
    std::condition_variable_any condition;

    {
        CooperativeThread worker;
        worker.start([&](std::stop_token stopToken) {
            started.set_value();

            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, stopToken, [] { return false; });
            observedStop.store(stopToken.stop_requested());
        });

        ASSERT_EQ(ready.wait_for(1s), std::future_status::ready);
    }

    EXPECT_TRUE(observedStop.load());
}

TEST(CooperativeThreadTest, RejectsASecondActiveWorker) {
    std::promise<void> started;
    std::future<void> ready = started.get_future();
    std::mutex mutex;
    std::condition_variable_any condition;
    CooperativeThread worker;

    worker.start([&](std::stop_token stopToken) {
        started.set_value();

        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, stopToken, [] { return false; });
    });

    ASSERT_EQ(ready.wait_for(1s), std::future_status::ready);
    EXPECT_THROW(worker.start([](std::stop_token) {}), std::logic_error);

    EXPECT_TRUE(worker.requestStop());
    worker.join();
    EXPECT_FALSE(worker.joinable());
}
