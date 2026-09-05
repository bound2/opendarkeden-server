#include <atomic>
#include <barrier>
#include <chrono>
#include <future>
#include <stdexcept>

#include <gtest/gtest.h>
#include <system_error>

#include "ManagedThread.h"
#include "gameserver/ThreadPool.h"

using namespace std::chrono_literals;

namespace {
class Worker : public ManagedThread {
public:
    ~Worker() noexcept override {
        stop();
        join();
    }
    std::promise<void> entered;
    std::atomic<bool> exited{false};
    bool failStart = false;
    bool failRun = false;

    void start() override {
        if (failStart)
            throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
        ManagedThread::start();
    }

    void run() override {
        entered.set_value();
        if (failRun)
            throw std::runtime_error("injected worker failure");
        while (pauseFor(1ms)) {
        }
        exited = true;
    }
};

class ManagedThreadTest : public testing::Test {
    void SetUp() override {
        ServerShutdown::requested = false;
        ServerShutdown::failed = false;
    }
    void TearDown() override {
        ServerShutdown::requested = false;
        ServerShutdown::failed = false;
    }
};
} // namespace

TEST_F(ManagedThreadTest, StopBeforeStartIsTerminal) {
    Worker worker;
    worker.stop();
    EXPECT_EQ(worker.getStatus(), Thread::EXIT);
    EXPECT_THROW(worker.start(), ThreadException);
    worker.join();
}

TEST_F(ManagedThreadTest, ConcurrentStartStopCannotLoseCancellation) {
    for (int i = 0; i != 200; ++i) {
        Worker worker;
        std::barrier ready(4);
        std::jthread starter([&] {
            ready.arrive_and_wait();
            try {
                worker.start();
            } catch (ThreadException&) {
            }
        });
        std::jthread stopper([&] {
            ready.arrive_and_wait();
            worker.stop();
        });
        std::jthread joiner([&] {
            ready.arrive_and_wait();
            worker.join();
        });
        ready.arrive_and_wait();
        starter.join();
        stopper.join();
        joiner.join();
        worker.join();
        EXPECT_EQ(worker.getStatus(), Thread::EXIT);
    }
}

TEST_F(ManagedThreadTest, StopCanInterruptAnAlreadyWaitingJoin) {
    Worker worker;
    auto entered = worker.entered.get_future();
    worker.start();
    ASSERT_EQ(entered.wait_for(1s), std::future_status::ready);
    auto joined = std::async(std::launch::async, [&] { Thread::join(worker); });
    worker.stop();
    EXPECT_EQ(joined.wait_for(1s), std::future_status::ready);
    EXPECT_TRUE(worker.exited);
    EXPECT_EQ(worker.getStatus(), Thread::EXIT);
}

TEST_F(ManagedThreadTest, WorkerFailureRequestsShutdownAndIsRetained) {
    Worker worker;
    worker.failRun = true;
    worker.start();
    worker.join();
    EXPECT_TRUE(ServerShutdown::isRequested());
    EXPECT_TRUE(ServerShutdown::failed.load());
    EXPECT_EQ(worker.getStatus(), Thread::EXIT);
    EXPECT_THROW(worker.rethrowFailure(), std::runtime_error);
}

TEST_F(ManagedThreadTest, FailedPoolStartupJoinsPrefixAndReleasesPoolLock) {
    ThreadPool pool;
    auto* first = new Worker;
    auto* second = new Worker;
    second->failStart = true;
    pool.addThread(first);
    pool.addThread(second);
    EXPECT_THROW(pool.start(), std::system_error);
    EXPECT_TRUE(ServerShutdown::isRequested());
    EXPECT_TRUE(ServerShutdown::failed.load());
    EXPECT_EQ(first->getStatus(), Thread::EXIT);
    EXPECT_EQ(second->getStatus(), Thread::EXIT);
    // Previously this reentrant stop deadlocked on the leaked startup lock.
    pool.stop();
}

TEST_F(ManagedThreadTest, PoolRequestsEveryStopBeforeAnyJoin) {
    struct OrderedWorker : Worker {
        explicit OrderedWorker(int& stopped) : count(stopped) {}
        int& count;
        bool recorded = false;
        void stop() override {
            if (!recorded) {
                ++count;
                recorded = true;
            }
            Worker::stop();
        }
        void join() override {
            EXPECT_EQ(count, 2);
            Worker::join();
        }
    };
    int count = 0;
    ThreadPool pool;
    pool.addThread(new OrderedWorker(count));
    pool.addThread(new OrderedWorker(count));
    pool.start();
    pool.stop();
}

TEST_F(ManagedThreadTest, OwningPoolJoinsBeforeExternalDependencyIsDestroyed) {
    std::atomic<bool> dependencyAlive{true};
    std::atomic<bool> sawLiveDependency{false};
    struct DependentWorker : Worker {
        DependentWorker(std::atomic<bool>& alive, std::atomic<bool>& observed) : alive(alive), observed(observed) {}
        ~DependentWorker() noexcept override {
            stop();
            join();
        }
        std::atomic<bool>& alive;
        std::atomic<bool>& observed;
        void run() override {
            entered.set_value();
            while (pauseFor(1ms)) {
            }
            observed = alive.load();
        }
    };
    {
        ThreadPool pool;
        auto* worker = new DependentWorker(dependencyAlive, sawLiveDependency);
        auto entered = worker->entered.get_future();
        pool.addThread(worker);
        pool.start();
        ASSERT_EQ(entered.wait_for(1s), std::future_status::ready);
    }
    dependencyAlive = false;
    EXPECT_TRUE(sawLiveDependency);
}

TEST_F(ManagedThreadTest, TermSignalDrainsWorkers) {
    ASSERT_EXIT(
        {
            struct sigaction action {};
            action.sa_handler = ServerShutdown::request;
            sigemptyset(&action.sa_mask);
            if (sigaction(SIGTERM, &action, nullptr) != 0)
                std::_Exit(2);
            ServerShutdown::Deadline deadline(1s);
            Worker worker;
            auto entered = worker.entered.get_future();
            worker.start();
            entered.wait();
            raise(SIGTERM);
            worker.stop();
            worker.join();
            std::_Exit(worker.exited && !ServerShutdown::failed.load() ? 0 : 3);
        },
        testing::ExitedWithCode(0), "");
}

TEST_F(ManagedThreadTest, DeadlineTerminatesBlockedInitializationWithoutFreeingState) {
    ASSERT_EXIT(
        {
            ServerShutdown::Deadline deadline(50ms);
            CooperativeThread worker;
            worker.start([](std::stop_token) {
                for (;;)
                    std::this_thread::sleep_for(1s);
            });
            ServerShutdown::request();
            worker.requestStop();
            worker.join();
            std::_Exit(0);
        },
        testing::ExitedWithCode(EXIT_FAILURE), "shutdown deadline exceeded");
}
