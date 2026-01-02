#pragma once

#include "ui/UIPanel.hpp"
#include "physics/PhysicsEngine.hpp"

namespace pas::ui {

/**
 * @brief Control panel for simulation control.
 */
class ControlPanel : public UIPanel {
public:
    ControlPanel(physics::PhysicsEngine& engine);

    void draw() override;

    // UI state getters for main app
    bool isWireframeModeEnabled() const { return m_wireframeMode; }

private:
    physics::PhysicsEngine& m_engine;
    float m_timeScale = 1e6f;
    int m_integratorType = 2;  // Boris
    bool m_wireframeMode = false;
    bool m_showDemoWindow = false;
};

} // namespace pas::ui
