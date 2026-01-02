#include "ui/ControlPanel.hpp"
#include "physics/Constants.hpp"

namespace pas::ui {

ControlPanel::ControlPanel(physics::PhysicsEngine& engine)
    : UIPanel("Simulation Control")
    , m_engine(engine) {}

void ControlPanel::draw() {
    if (!m_visible) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 450), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(m_title.c_str(), &m_visible)) {
        ImGui::End();
        return;
    }

    // Simulation controls
    ImGui::Text("Simulation");
    ImGui::Separator();

    const auto& stats = m_engine.getStats();
    const char* stateStr = m_engine.isRunning() ? "Running" :
                           m_engine.isPaused() ? "Paused" : "Stopped";

    // Status with colored indicator
    ImVec4 statusColor = m_engine.isRunning() ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) :
                         m_engine.isPaused() ? ImVec4(0.9f, 0.7f, 0.0f, 1.0f) :
                                               ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    ImGui::TextColored(statusColor, "State: %s", stateStr);

    // Control buttons
    if (ImGui::Button(m_engine.isRunning() ? "Pause" : "Start", ImVec2(80, 0))) {
        if (m_engine.isRunning()) {
            m_engine.pause();
        } else if (m_engine.isPaused()) {
            m_engine.resume();
        } else {
            m_engine.start();
            m_engine.initializeDefaultBeam();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(80, 0))) {
        m_engine.reset();
        m_engine.initializeDefaultBeam();
    }

    ImGui::Spacing();

    // Time scale
    if (ImGui::SliderFloat("Time Scale", &m_timeScale, 1.0f, 1e9f, "%.0e",
                           ImGuiSliderFlags_Logarithmic)) {
        m_engine.setTimeScale(static_cast<double>(m_timeScale));
    }
    ImGui::SetItemTooltip("Simulation speed multiplier");

    // Integrator selection
    const char* integrators[] = { "Euler", "Velocity Verlet", "Boris", "RK4" };
    if (ImGui::Combo("Integrator", &m_integratorType, integrators, 4)) {
        m_engine.setIntegrator(static_cast<physics::IntegratorFactory::Type>(m_integratorType));
    }
    ImGui::SetItemTooltip("Numerical integration method");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Statistics");
    ImGui::Separator();

    // Statistics table
    if (ImGui::BeginTable("StatsTable", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Sim Time:");
        ImGui::TableNextColumn();
        ImGui::Text("%.6e s", stats.simulationTime);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Steps:");
        ImGui::TableNextColumn();
        ImGui::Text("%llu", stats.stepCount);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Steps/sec:");
        ImGui::TableNextColumn();
        ImGui::Text("%.0f", stats.stepsPerSecond);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Particles:");
        ImGui::TableNextColumn();
        ImGui::Text("%zu / %zu", stats.particleCount,
                    stats.particleCount + stats.lostParticleCount);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Avg Energy:");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f GeV", stats.averageEnergy / (1e9 * physics::constants::energy::eV));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Energy Spread:");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f MeV", stats.energySpread / (1e6 * physics::constants::energy::eV));

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Rendering");
    ImGui::Separator();

    ImGui::Checkbox("Wireframe Mode", &m_wireframeMode);
    ImGui::Checkbox("Show ImGui Demo", &m_showDemoWindow);

    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }

    ImGui::End();
}

} // namespace pas::ui
