#pragma once

#include <string>
#include <imgui.h>

namespace pas::ui {

/**
 * @brief Base class for UI panels.
 */
class UIPanel {
public:
    UIPanel(std::string title) : m_title(std::move(title)) {}
    virtual ~UIPanel() = default;

    /**
     * @brief Draw the panel.
     */
    virtual void draw() = 0;

    /**
     * @brief Check if panel is visible.
     */
    bool isVisible() const { return m_visible; }

    /**
     * @brief Set panel visibility.
     */
    void setVisible(bool visible) { m_visible = visible; }

    /**
     * @brief Toggle panel visibility.
     */
    void toggleVisible() { m_visible = !m_visible; }

    /**
     * @brief Get panel title.
     */
    const std::string& getTitle() const { return m_title; }

protected:
    std::string m_title;
    bool m_visible = true;
};

} // namespace pas::ui
