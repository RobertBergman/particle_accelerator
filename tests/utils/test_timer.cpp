#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "utils/Timer.hpp"

namespace pas::utils::tests {

class TimerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TimerTest, StartsRunningByDefault) {
    Timer timer;
    EXPECT_TRUE(timer.isRunning());
}

TEST_F(TimerTest, StopActuallyStops) {
    Timer timer;
    EXPECT_TRUE(timer.isRunning());
    timer.stop();
    EXPECT_FALSE(timer.isRunning());
}

TEST_F(TimerTest, ResetRestartsTimer) {
    Timer timer;
    timer.stop();
    EXPECT_FALSE(timer.isRunning());
    timer.reset();
    EXPECT_TRUE(timer.isRunning());
}

TEST_F(TimerTest, ElapsedTimeIncreasesWhileRunning) {
    Timer timer;

    // Wait a small amount
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    double elapsed = timer.elapsedMilliseconds();
    EXPECT_GT(elapsed, 0.0);
    EXPECT_LT(elapsed, 1000.0); // Sanity check
}

TEST_F(TimerTest, ElapsedTimeStopsWhenStopped) {
    Timer timer;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timer.stop();

    double elapsed1 = timer.elapsedMilliseconds();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    double elapsed2 = timer.elapsedMilliseconds();

    // Elapsed time should not change while stopped
    EXPECT_DOUBLE_EQ(elapsed1, elapsed2);
}

TEST_F(TimerTest, ResumeAccumulatesTime) {
    Timer timer;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timer.stop();
    double elapsed1 = timer.elapsedMilliseconds();

    timer.resume();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timer.stop();
    double elapsed2 = timer.elapsedMilliseconds();

    EXPECT_GT(elapsed2, elapsed1);
}

TEST_F(TimerTest, TimeUnitConversions) {
    Timer timer;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    timer.stop();

    double seconds = timer.elapsedSeconds();
    double milliseconds = timer.elapsedMilliseconds();
    double microseconds = timer.elapsedMicroseconds();
    double nanoseconds = timer.elapsedNanoseconds();

    // Check conversions are consistent
    EXPECT_NEAR(seconds * 1000.0, milliseconds, 0.001);
    EXPECT_NEAR(milliseconds * 1000.0, microseconds, 1.0);
    EXPECT_NEAR(microseconds * 1000.0, nanoseconds, 1000.0);

    // Check reasonable range
    EXPECT_GT(milliseconds, 40.0);
    EXPECT_LT(milliseconds, 200.0);
}

// FrameTimer tests

class FrameTimerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(FrameTimerTest, InitialStateIsValid) {
    FrameTimer timer;

    EXPECT_EQ(timer.getFrameCount(), 0u);
    EXPECT_DOUBLE_EQ(timer.getDeltaTime(), 0.0);
    EXPECT_GT(timer.getFPS(), 0.0);
}

TEST_F(FrameTimerTest, TickIncreasesFrameCount) {
    FrameTimer timer;

    timer.tick();
    EXPECT_EQ(timer.getFrameCount(), 1u);

    timer.tick();
    EXPECT_EQ(timer.getFrameCount(), 2u);
}

TEST_F(FrameTimerTest, DeltaTimeReturnsPositiveValue) {
    FrameTimer timer;

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    timer.tick();

    double dt = timer.getDeltaTime();
    EXPECT_GT(dt, 0.0);
    EXPECT_LT(dt, 1.0);
}

TEST_F(FrameTimerTest, TotalTimeAccumulates) {
    FrameTimer timer;

    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timer.tick();
    }

    double totalTime = timer.getTotalTime();
    EXPECT_GT(totalTime, 0.04); // At least 40ms
    EXPECT_LT(totalTime, 1.0);  // Less than 1 second
}

TEST_F(FrameTimerTest, DeltaTimeIsClamped) {
    FrameTimer timer;

    // Simulate a long delay (spiral of death protection)
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    timer.tick();

    // Delta time should be clamped to 0.25 seconds
    EXPECT_LE(timer.getDeltaTime(), 0.25);
}

} // namespace pas::utils::tests
