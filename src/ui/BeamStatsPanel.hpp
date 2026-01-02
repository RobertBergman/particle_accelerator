#pragma once

#include "ui/UIPanel.hpp"
#include "physics/ParticleSystem.hpp"
#include <vector>
#include <array>

namespace pas::ui {

/**
 * @brief Panel displaying real-time beam statistics and plots.
 */
class BeamStatsPanel : public UIPanel {
public:
    BeamStatsPanel(const physics::ParticleSystem& particleSystem);

    void draw() override;

    /**
     * @brief Update statistics (call each frame).
     */
    void update();

private:
    void drawPositionHistogram();
    void drawMomentumHistogram();
    void drawEnergyHistory();
    void drawEmittancePlot();

    const physics::ParticleSystem& m_particleSystem;

    // History buffers for plots
    static constexpr size_t HISTORY_SIZE = 200;
    std::array<float, HISTORY_SIZE> m_energyHistory{};
    std::array<float, HISTORY_SIZE> m_emittanceXHistory{};
    std::array<float, HISTORY_SIZE> m_emittanceYHistory{};
    size_t m_historyIndex = 0;

    // Histogram data
    static constexpr size_t NUM_BINS = 50;
    std::array<float, NUM_BINS> m_xHistogram{};
    std::array<float, NUM_BINS> m_yHistogram{};
    std::array<float, NUM_BINS> m_pxHistogram{};
    std::array<float, NUM_BINS> m_pyHistogram{};
};

} // namespace pas::ui
