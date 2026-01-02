#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace pas::utils {

/**
 * @brief High-resolution timer for performance measurement and simulation timing.
 *
 * Uses std::chrono::high_resolution_clock for maximum precision.
 */
class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<double>;

    /**
     * @brief Construct a new Timer. Automatically starts timing.
     */
    Timer();

    /**
     * @brief Reset and start the timer.
     */
    void reset();

    /**
     * @brief Start the timer (if stopped).
     */
    void start();

    /**
     * @brief Stop the timer.
     */
    void stop();

    /**
     * @brief Resume a stopped timer.
     */
    void resume();

    /**
     * @brief Check if the timer is currently running.
     */
    bool isRunning() const;

    /**
     * @brief Get elapsed time in seconds.
     */
    double elapsedSeconds() const;

    /**
     * @brief Get elapsed time in milliseconds.
     */
    double elapsedMilliseconds() const;

    /**
     * @brief Get elapsed time in microseconds.
     */
    double elapsedMicroseconds() const;

    /**
     * @brief Get elapsed time in nanoseconds.
     */
    double elapsedNanoseconds() const;

    /**
     * @brief Get the raw elapsed duration.
     */
    Duration elapsed() const;

private:
    TimePoint m_startTime;
    Duration m_accumulatedTime;
    bool m_running;
};

/**
 * @brief Scoped timer for automatic timing of code blocks.
 *
 * Logs the elapsed time when the object goes out of scope.
 */
class ScopedTimer {
public:
    /**
     * @brief Construct a scoped timer with a name for identification.
     * @param name Name to display in the log output.
     */
    explicit ScopedTimer(std::string name);

    /**
     * @brief Destructor logs the elapsed time.
     */
    ~ScopedTimer();

    // Non-copyable
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
    std::string m_name;
    Timer m_timer;
};

/**
 * @brief Frame timer for tracking frame times and calculating delta time.
 */
class FrameTimer {
public:
    using TimePoint = Timer::TimePoint;

    /**
     * @brief Construct a new frame timer.
     */
    FrameTimer();

    /**
     * @brief Call at the start of each frame to update timing.
     * @return Delta time since last frame in seconds.
     */
    double tick();

    /**
     * @brief Get the delta time from the last tick.
     */
    double getDeltaTime() const;

    /**
     * @brief Get the current frames per second (smoothed).
     */
    double getFPS() const;

    /**
     * @brief Get the total elapsed time since the timer was created.
     */
    double getTotalTime() const;

    /**
     * @brief Get the current frame number.
     */
    uint64_t getFrameCount() const;

private:
    static constexpr double FPS_SMOOTHING = 0.9;

    Timer m_timer;
    TimePoint m_lastFrameTime;
    double m_deltaTime;
    double m_smoothedFPS;
    double m_totalTime;
    uint64_t m_frameCount;
};

} // namespace pas::utils

// Convenience macro for scoped timing
#define PAS_SCOPED_TIMER(name) ::pas::utils::ScopedTimer _scopedTimer##__LINE__(name)
