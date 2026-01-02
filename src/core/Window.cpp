#include "core/Window.hpp"
#include "utils/Logger.hpp"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace pas::core {

Window::Window() : m_config() {}

Window::Window(const WindowConfig& config) : m_config(config) {}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

Window::Window(Window&& other) noexcept
    : m_window(other.m_window)
    , m_config(std::move(other.m_config))
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_cursorCaptured(other.m_cursorCaptured)
    , m_resizeCallback(std::move(other.m_resizeCallback))
    , m_keyCallback(std::move(other.m_keyCallback))
    , m_mouseButtonCallback(std::move(other.m_mouseButtonCallback))
    , m_mouseMoveCallback(std::move(other.m_mouseMoveCallback))
    , m_scrollCallback(std::move(other.m_scrollCallback))
{
    other.m_window = nullptr;
    if (m_window) {
        glfwSetWindowUserPointer(m_window, this);
    }
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (m_window) {
            glfwDestroyWindow(m_window);
        }
        m_window = other.m_window;
        m_config = std::move(other.m_config);
        m_width = other.m_width;
        m_height = other.m_height;
        m_cursorCaptured = other.m_cursorCaptured;
        m_resizeCallback = std::move(other.m_resizeCallback);
        m_keyCallback = std::move(other.m_keyCallback);
        m_mouseButtonCallback = std::move(other.m_mouseButtonCallback);
        m_mouseMoveCallback = std::move(other.m_mouseMoveCallback);
        m_scrollCallback = std::move(other.m_scrollCallback);
        other.m_window = nullptr;
        if (m_window) {
            glfwSetWindowUserPointer(m_window, this);
        }
    }
    return *this;
}

bool Window::init() {
    // Initialize GLFW if not already done
    if (!glfwInit()) {
        PAS_ERROR("Failed to initialize GLFW");
        return false;
    }

    // Set OpenGL version hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, m_config.samples);

#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    // Create window
    GLFWmonitor* monitor = m_config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    m_window = glfwCreateWindow(
        m_config.width,
        m_config.height,
        m_config.title.c_str(),
        monitor,
        nullptr
    );

    if (!m_window) {
        PAS_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    // Make context current
    glfwMakeContextCurrent(m_window);

    // Load OpenGL functions
    if (!gladLoadGL()) {
        PAS_ERROR("Failed to initialize GLAD");
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        return false;
    }

    // Store window dimensions
    glfwGetWindowSize(m_window, &m_width, &m_height);

    // Set user pointer for callbacks
    glfwSetWindowUserPointer(m_window, this);

    // Set callbacks
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetKeyCallback(m_window, keyCallbackStatic);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallbackStatic);
    glfwSetCursorPosCallback(m_window, cursorPosCallbackStatic);
    glfwSetScrollCallback(m_window, scrollCallbackStatic);

    // Enable VSync
    setVSync(m_config.vsync);

    // Log OpenGL info
    PAS_INFO("OpenGL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    PAS_INFO("OpenGL Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    return true;
}

bool Window::shouldClose() const {
    return m_window && glfwWindowShouldClose(m_window);
}

void Window::close() {
    if (m_window) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

glm::ivec2 Window::getFramebufferSize() const {
    int width = 0, height = 0;
    if (m_window) {
        glfwGetFramebufferSize(m_window, &width, &height);
    }
    return {width, height};
}

float Window::getAspectRatio() const {
    if (m_height == 0) return 1.0f;
    return static_cast<float>(m_width) / static_cast<float>(m_height);
}

void Window::setTitle(const std::string& title) {
    m_config.title = title;
    if (m_window) {
        glfwSetWindowTitle(m_window, title.c_str());
    }
}

void Window::setVSync(bool enabled) {
    m_config.vsync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::setCursorCapture(bool captured) {
    m_cursorCaptured = captured;
    if (m_window) {
        glfwSetInputMode(m_window, GLFW_CURSOR,
            captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

bool Window::isKeyPressed(int key) const {
    if (!m_window) return false;
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    if (!m_window) return false;
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

glm::dvec2 Window::getMousePosition() const {
    double x = 0.0, y = 0.0;
    if (m_window) {
        glfwGetCursorPos(m_window, &x, &y);
    }
    return {x, y};
}

// Static callback implementations
void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_width = width;
        self->m_height = height;
        glViewport(0, 0, width, height);
        if (self->m_resizeCallback) {
            self->m_resizeCallback(width, height);
        }
    }
}

void Window::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_keyCallback) {
        self->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_mouseButtonCallback) {
        self->m_mouseButtonCallback(button, action, mods);
    }
}

void Window::cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_mouseMoveCallback) {
        self->m_mouseMoveCallback(xpos, ypos);
    }
}

void Window::scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_scrollCallback) {
        self->m_scrollCallback(xoffset, yoffset);
    }
}

} // namespace pas::core
