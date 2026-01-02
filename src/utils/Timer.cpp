#include "utils/Timer.hpp"
#include "utils/Logger.hpp"

namespace pas::utils {

// Timer implementation

Timer::Timer()
    : m_startTime(Clock::now())
    , m_accumulatedTime(Duration::zero())
    , m_running(true) {
}

void Timer::reset() {
    m_startTime = Clock::now();
    m_accumulatedTime = Duration::zero();
    m_running = true;
}

void Timer::start() {
    if (!m_running) {
        m_startTime = Clock::now();
        m_running = true;
    }
}

void Timer::stop() {
    if (m_running) {
        auto now = Clock::now();
        m_accumulatedTime += std::chrono::duration_cast<Duration>(now - m_startTime);
        m_running = false;
    }
}

void Timer::resume() {
    if (!m_running) {
        m_startTime = Clock::now();
        m_running = true;
    }
}

bool Timer::isRunning() const {
    return m_running;
}

double Timer::elapsedSeconds() const {
    return elapsed().count();
}

double Timer::elapsedMilliseconds() const {
    return elapsedSeconds() * 1000.0;
}

double Timer::elapsedMicroseconds() const {
    return elapsedSeconds() * 1'000'000.0;
}

double Timer::elapsedNanoseconds() const {
    return elapsedSeconds() * 1'000'000'000.0;
}

Timer::Duration Timer::elapsed() const {
    Duration total = m_accumulatedTime;
    if (m_running) {
        auto now = Clock::now();
        total += std::chrono::duration_cast<Duration>(now - m_startTime);
    }
    return total;
}

// ScopedTimer implementation

ScopedTimer::ScopedTimer(std::string name)
    : m_name(std::move(name))
    , m_timer() {
}

ScopedTimer::~ScopedTimer() {
    double elapsed = m_timer.elapsedMilliseconds();
    PAS_DEBUG("[Timer] {} took {:.3f} ms", m_name, elapsed);
}

// FrameTimer implementation

FrameTimer::FrameTimer()
    : m_timer()
    , m_lastFrameTime(Timer::Clock::now())
    , m_deltaTime(0.0)
    , m_smoothedFPS(60.0)
    , m_totalTime(0.0)
    , m_frameCount(0) {
}

double FrameTimer::tick() {
    auto now = Timer::Clock::now();
    auto duration = std::chrono::duration_cast<Timer::Duration>(now - m_lastFrameTime);
    m_deltaTime = duration.count();
    m_lastFrameTime = now;

    // Clamp delta time to avoid spiral of death
    if (m_deltaTime > 0.25) {
        m_deltaTime = 0.25;
    }

    m_totalTime += m_deltaTime;
    m_frameCount++;

    // Smooth FPS calculation
    if (m_deltaTime > 0.0) {
        double instantFPS = 1.0 / m_deltaTime;
        m_smoothedFPS = FPS_SMOOTHING * m_smoothedFPS + (1.0 - FPS_SMOOTHING) * instantFPS;
    }

    return m_deltaTime;
}

double FrameTimer::getDeltaTime() const {
    return m_deltaTime;
}

double FrameTimer::getFPS() const {
    return m_smoothedFPS;
}

double FrameTimer::getTotalTime() const {
    return m_totalTime;
}

uint64_t FrameTimer::getFrameCount() const {
    return m_frameCount;
}

} // namespace pas::utils
