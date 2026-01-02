#include "ui/BeamStatsPanel.hpp"
#include "physics/Constants.hpp"
#include <algorithm>
#include <cmath>

namespace pas::ui {

BeamStatsPanel::BeamStatsPanel(const physics::ParticleSystem& particleSystem)
    : UIPanel("Beam Diagnostics")
    , m_particleSystem(particleSystem) {}

void BeamStatsPanel::update() {
    auto stats = m_particleSystem.computeStatistics();

    // Update history buffers
    m_energyHistory[m_historyIndex] = static_cast<float>(
        stats.meanEnergy / (1e9 * physics::constants::energy::eV));  // GeV
    m_emittanceXHistory[m_historyIndex] = static_cast<float>(
        stats.normalizedEmittanceX * 1e6);  // mm-mrad
    m_emittanceYHistory[m_historyIndex] = static_cast<float>(
        stats.normalizedEmittanceY * 1e6);  // mm-mrad

    m_historyIndex = (m_historyIndex + 1) % HISTORY_SIZE;

    // Update histograms
    std::fill(m_xHistogram.begin(), m_xHistogram.end(), 0.0f);
    std::fill(m_yHistogram.begin(), m_yHistogram.end(), 0.0f);
    std::fill(m_pxHistogram.begin(), m_pxHistogram.end(), 0.0f);
    std::fill(m_pyHistogram.begin(), m_pyHistogram.end(), 0.0f);

    const auto& particles = m_particleSystem.getParticles();
    if (particles.empty()) return;

    // Find ranges for histograms
    double xMin = 1e10, xMax = -1e10;
    double yMin = 1e10, yMax = -1e10;
    double pxMin = 1e10, pxMax = -1e10;
    double pyMin = 1e10, pyMax = -1e10;

    for (const auto& p : particles) {
        if (!p.isActive()) continue;
        auto pos = p.getPosition();
        auto mom = p.getMomentum();

        xMin = std::min(xMin, pos.x);
        xMax = std::max(xMax, pos.x);
        yMin = std::min(yMin, pos.y);
        yMax = std::max(yMax, pos.y);
        pxMin = std::min(pxMin, mom.x);
        pxMax = std::max(pxMax, mom.x);
        pyMin = std::min(pyMin, mom.y);
        pyMax = std::max(pyMax, mom.y);
    }

    // Fill histograms
    double xRange = std::max(xMax - xMin, 1e-10);
    double yRange = std::max(yMax - yMin, 1e-10);
    double pxRange = std::max(pxMax - pxMin, 1e-10);
    double pyRange = std::max(pyMax - pyMin, 1e-10);

    for (const auto& p : particles) {
        if (!p.isActive()) continue;
        auto pos = p.getPosition();
        auto mom = p.getMomentum();

        size_t xBin = std::min(static_cast<size_t>((pos.x - xMin) / xRange * NUM_BINS), NUM_BINS - 1);
        size_t yBin = std::min(static_cast<size_t>((pos.y - yMin) / yRange * NUM_BINS), NUM_BINS - 1);
        size_t pxBin = std::min(static_cast<size_t>((mom.x - pxMin) / pxRange * NUM_BINS), NUM_BINS - 1);
        size_t pyBin = std::min(static_cast<size_t>((mom.y - pyMin) / pyRange * NUM_BINS), NUM_BINS - 1);

        m_xHistogram[xBin] += 1.0f;
        m_yHistogram[yBin] += 1.0f;
        m_pxHistogram[pxBin] += 1.0f;
        m_pyHistogram[pyBin] += 1.0f;
    }
}

void BeamStatsPanel::draw() {
    if (!m_visible) return;

    ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(m_title.c_str(), &m_visible)) {
        ImGui::End();
        return;
    }

    auto stats = m_particleSystem.computeStatistics();

    // Beam statistics
    ImGui::Text("Beam Parameters");
    ImGui::Separator();

    if (ImGui::BeginTable("BeamParams", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Active Particles:");
        ImGui::TableNextColumn();
        ImGui::Text("%zu", stats.activeParticles);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Mean Position:");
        ImGui::TableNextColumn();
        ImGui::Text("(%.2f, %.2f, %.2f) mm",
                    stats.meanPosition.x * 1000,
                    stats.meanPosition.y * 1000,
                    stats.meanPosition.z * 1000);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("RMS Beam Size:");
        ImGui::TableNextColumn();
        ImGui::Text("(%.3f, %.3f) mm",
                    stats.rmsSize.x * 1000,
                    stats.rmsSize.y * 1000);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Mean Energy:");
        ImGui::TableNextColumn();
        ImGui::Text("%.4f GeV", stats.meanEnergy / (1e9 * physics::constants::energy::eV));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Energy Spread:");
        ImGui::TableNextColumn();
        ImGui::Text("%.4f %%", (stats.rmsEnergy / stats.meanEnergy) * 100.0);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Emittance (norm):");
        ImGui::TableNextColumn();
        ImGui::Text("(%.3f, %.3f) mm-mrad",
                    stats.normalizedEmittanceX * 1e6,
                    stats.normalizedEmittanceY * 1e6);

        ImGui::EndTable();
    }

    ImGui::Spacing();

    // Plots in tabs
    if (ImGui::BeginTabBar("DiagnosticsTab")) {
        if (ImGui::BeginTabItem("Energy")) {
            drawEnergyHistory();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Position")) {
            drawPositionHistogram();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Momentum")) {
            drawMomentumHistogram();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Emittance")) {
            drawEmittancePlot();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void BeamStatsPanel::drawEnergyHistory() {
    ImGui::Text("Mean Energy History");
    ImGui::PlotLines("##EnergyHistory", m_energyHistory.data(), HISTORY_SIZE,
                     static_cast<int>(m_historyIndex), "GeV",
                     FLT_MAX, FLT_MAX, ImVec2(-1, 100));
}

void BeamStatsPanel::drawPositionHistogram() {
    ImGui::Text("X Distribution");
    ImGui::PlotHistogram("##XHist", m_xHistogram.data(), NUM_BINS,
                         0, nullptr, 0.0f, FLT_MAX, ImVec2(-1, 80));

    ImGui::Text("Y Distribution");
    ImGui::PlotHistogram("##YHist", m_yHistogram.data(), NUM_BINS,
                         0, nullptr, 0.0f, FLT_MAX, ImVec2(-1, 80));
}

void BeamStatsPanel::drawMomentumHistogram() {
    ImGui::Text("Px Distribution");
    ImGui::PlotHistogram("##PxHist", m_pxHistogram.data(), NUM_BINS,
                         0, nullptr, 0.0f, FLT_MAX, ImVec2(-1, 80));

    ImGui::Text("Py Distribution");
    ImGui::PlotHistogram("##PyHist", m_pyHistogram.data(), NUM_BINS,
                         0, nullptr, 0.0f, FLT_MAX, ImVec2(-1, 80));
}

void BeamStatsPanel::drawEmittancePlot() {
    ImGui::Text("Normalized Emittance History (mm-mrad)");

    ImGui::PlotLines("X##EmitX", m_emittanceXHistory.data(), HISTORY_SIZE,
                     static_cast<int>(m_historyIndex), "X",
                     FLT_MAX, FLT_MAX, ImVec2(-1, 80));

    ImGui::PlotLines("Y##EmitY", m_emittanceYHistory.data(), HISTORY_SIZE,
                     static_cast<int>(m_historyIndex), "Y",
                     FLT_MAX, FLT_MAX, ImVec2(-1, 80));
}

} // namespace pas::ui
