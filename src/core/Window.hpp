#pragma once

#include <string>
#include <functional>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace pas::core {

/**
 * @brief Window configuration parameters.
 */
struct WindowConfig {
    int width = 1280;
    int height = 720;
    std::string title = "Particle Accelerator Simulation";
    bool vsync = true;
    bool fullscreen = false;
    int samples = 4;  // MSAA samples
};

/**
 * @brief Callback types for window events.
 */
using ResizeCallback = std::function<void(int, int)>;
using KeyCallback = std::function<void(int, int, int, int)>;
using MouseButtonCallback = std::function<void(int, int, int)>;
using MouseMoveCallback = std::function<void(double, double)>;
using ScrollCallback = std::function<void(double, double)>;

/**
 * @brief GLFW window wrapper with OpenGL context management.
 *
 * Handles window creation, input callbacks, and OpenGL context setup.
 */
class Window {
public:
    Window();
    explicit Window(const WindowConfig& config);
    ~Window();

    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Movable
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    /**
     * @brief Initialize the window and OpenGL context.
     * @return True if initialization succeeded.
     */
    bool init();

    /**
     * @brief Check if the window should close.
     */
    bool shouldClose() const;

    /**
     * @brief Request window close.
     */
    void close();

    /**
     * @brief Poll window events.
     */
    void pollEvents();

    /**
     * @brief Swap front and back buffers.
     */
    void swapBuffers();

    /**
     * @brief Get window dimensions.
     */
    glm::ivec2 getSize() const { return {m_width, m_height}; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    /**
     * @brief Get framebuffer dimensions (may differ on HiDPI).
     */
    glm::ivec2 getFramebufferSize() const;

    /**
     * @brief Get aspect ratio.
     */
    float getAspectRatio() const;

    /**
     * @brief Check if window is valid.
     */
    bool isValid() const { return m_window != nullptr; }

    /**
     * @brief Get the underlying GLFW window handle.
     */
    GLFWwindow* getHandle() const { return m_window; }

    /**
     * @brief Set window title.
     */
    void setTitle(const std::string& title);

    /**
     * @brief Enable/disable VSync.
     */
    void setVSync(bool enabled);

    /**
     * @brief Enable/disable cursor capture.
     */
    void setCursorCapture(bool captured);

    /**
     * @brief Check if cursor is captured.
     */
    bool isCursorCaptured() const { return m_cursorCaptured; }

    // Callback setters
    void setResizeCallback(ResizeCallback callback) { m_resizeCallback = std::move(callback); }
    void setKeyCallback(KeyCallback callback) { m_keyCallback = std::move(callback); }
    void setMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = std::move(callback); }
    void setMouseMoveCallback(MouseMoveCallback callback) { m_mouseMoveCallback = std::move(callback); }
    void setScrollCallback(ScrollCallback callback) { m_scrollCallback = std::move(callback); }

    // Input state queries
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    glm::dvec2 getMousePosition() const;

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);

    GLFWwindow* m_window = nullptr;
    WindowConfig m_config;
    int m_width = 0;
    int m_height = 0;
    bool m_cursorCaptured = false;

    // Callbacks
    ResizeCallback m_resizeCallback;
    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    MouseMoveCallback m_mouseMoveCallback;
    ScrollCallback m_scrollCallback;
};

} // namespace pas::core
