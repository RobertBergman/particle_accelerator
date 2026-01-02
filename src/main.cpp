#include "utils/Logger.hpp"
#include "utils/Timer.hpp"
#include "physics/Constants.hpp"
#include "physics/PhysicsEngine.hpp"
#include "accelerator/Accelerator.hpp"
#include "core/Window.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Camera.hpp"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <memory>
#include <iostream>

using namespace pas;

/**
 * @brief Create a demo FODO lattice for testing.
 */
std::shared_ptr<accelerator::Accelerator> createDemoAccelerator() {
    auto acc = std::make_shared<accelerator::Accelerator>();

    // Create a simple FODO cell using the built-in helper
    accelerator::FODOCellParams params;
    params.cellLength = 10.0;      // 10 meter cell
    params.quadLength = 0.5;       // 0.5m quadrupoles
    params.quadGradient = 20.0;    // 20 T/m gradient
    params.aperture = 0.05;        // 5 cm aperture

    acc->buildFODOLattice(params, 4);  // Build 4 FODO cells

    // Add an RF Cavity
    accelerator::Aperture rfAperture;
    rfAperture.radiusX = 0.05;
    rfAperture.radiusY = 0.05;
    auto rf = std::make_shared<accelerator::RFCavity>("RF1", 1.0, 1e6, 500e6, 0.0, rfAperture);
    acc->addComponent(rf);

    // Compute the lattice (calculate s-positions)
    acc->computeLattice();

    PAS_INFO("Created demo accelerator");
    PAS_INFO("  Total length: {:.2f} m", acc->getTotalLength());
    PAS_INFO("  Components: {}", acc->getComponentCount());

    return acc;
}

int main() {
    // Initialize logging
    utils::Logger::init("PAS", utils::Logger::Level::Debug);

    PAS_INFO("==============================================");
    PAS_INFO("  Particle Accelerator Simulation v1.0.0");
    PAS_INFO("==============================================");

    // Create window
    core::WindowConfig config;
    config.title = "Particle Accelerator Simulation";
    config.width = 1600;
    config.height = 900;
    config.vsync = true;
    config.samples = 4;

    core::Window window(config);
    if (!window.init()) {
        PAS_CRITICAL("Failed to create window!");
        return -1;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.getHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 450");

    // Create physics engine
    physics::PhysicsEngine physicsEngine;
    physicsEngine.setTimeStep(1e-10);  // 0.1 ns timestep
    physicsEngine.setTimeScale(1e4);   // Speed up simulation (10k x real-time)
    physicsEngine.setMaxStepsPerFrame(10000);  // Cap steps to keep UI responsive

    // Create and set accelerator
    auto accelerator = createDemoAccelerator();
    physicsEngine.setAccelerator(accelerator);

    // Initialize beam
    physicsEngine.initializeDefaultBeam();

    // Create renderer
    rendering::Renderer renderer;
    if (!renderer.initialize(window.getWidth(), window.getHeight())) {
        PAS_CRITICAL("Failed to initialize renderer!");
        return -1;
    }

    // Setup camera
    rendering::Camera camera(static_cast<float>(window.getWidth()) / window.getHeight());
    camera.setMode(rendering::CameraMode::Orbit);
    camera.setTarget({accelerator->getTotalLength() / 2.0f, 0.0f, 0.0f});
    camera.setOrbitDistance(20.0f);

    // UI state
    bool showDemoWindow = false;
    bool showStatsOverlay = true;
    float timeScale = 1e4f;  // Match physics engine default
    int integratorType = 2;  // Boris
    bool wireframeMode = false;

    // Mouse state for camera control
    bool rightMouseDown = false;
    double lastMouseX = 0.0, lastMouseY = 0.0;

    // Set up mouse callbacks
    window.setMouseButtonCallback([&](int button, int action, int /*mods*/) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            rightMouseDown = (action == GLFW_PRESS);
            if (rightMouseDown) {
                glfwGetCursorPos(window.getHandle(), &lastMouseX, &lastMouseY);
            }
        }
    });

    window.setScrollCallback([&](double /*xoffset*/, double yoffset) {
        camera.zoom(static_cast<float>(yoffset) * 2.0f);
    });

    // Frame timing
    utils::Timer frameTimer;
    double lastFrameTime = 0.0;

    PAS_INFO("Entering main loop...");
    PAS_INFO("Controls:");
    PAS_INFO("  Right-click + drag: Orbit camera");
    PAS_INFO("  Scroll: Zoom in/out");
    PAS_INFO("  Space: Start/Pause simulation");
    PAS_INFO("  R: Reset simulation");
    PAS_INFO("  ESC: Exit");

    // Main loop
    while (!window.shouldClose()) {
        // Calculate delta time
        double currentTime = frameTimer.elapsedSeconds();
        double deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // Poll events
        window.pollEvents();

        // Handle camera input
        if (rightMouseDown) {
            double mouseX, mouseY;
            glfwGetCursorPos(window.getHandle(), &mouseX, &mouseY);
            float dx = static_cast<float>(mouseX - lastMouseX);
            float dy = static_cast<float>(mouseY - lastMouseY);
            lastMouseX = mouseX;
            lastMouseY = mouseY;

            camera.orbit(dx, dy);
        }

        // Keyboard controls
        if (glfwGetKey(window.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            window.close();
        }
        if (glfwGetKey(window.getHandle(), GLFW_KEY_SPACE) == GLFW_PRESS) {
            static bool spacePressed = false;
            if (!spacePressed) {
                if (physicsEngine.isRunning()) {
                    physicsEngine.pause();
                } else if (physicsEngine.isPaused()) {
                    physicsEngine.resume();
                } else {
                    physicsEngine.start();
                }
                spacePressed = true;
            }
        } else {
            static bool spacePressed = false;
            spacePressed = false;
        }
        if (glfwGetKey(window.getHandle(), GLFW_KEY_R) == GLFW_PRESS) {
            physicsEngine.reset();
            physicsEngine.initializeDefaultBeam();
        }

        // Update physics
        physicsEngine.update(deltaTime);

        // Update camera
        camera.update();

        // Begin ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Control panel
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
            ImGui::Begin("Simulation Control");

            // Simulation controls
            ImGui::Text("Simulation");
            ImGui::Separator();

            const auto& stats = physicsEngine.getStats();
            const char* stateStr = physicsEngine.isRunning() ? "Running" :
                                   physicsEngine.isPaused() ? "Paused" : "Stopped";
            ImGui::Text("State: %s", stateStr);

            if (ImGui::Button(physicsEngine.isRunning() ? "Pause" : "Start")) {
                if (physicsEngine.isRunning()) {
                    physicsEngine.pause();
                } else if (physicsEngine.isPaused()) {
                    physicsEngine.resume();
                } else {
                    physicsEngine.start();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                physicsEngine.reset();
                physicsEngine.initializeDefaultBeam();
            }

            ImGui::Spacing();

            // Time scale
            if (ImGui::SliderFloat("Time Scale", &timeScale, 1.0f, 1e9f, "%.0e", ImGuiSliderFlags_Logarithmic)) {
                physicsEngine.setTimeScale(static_cast<double>(timeScale));
            }

            // Integrator selection
            const char* integrators[] = { "Euler", "Velocity Verlet", "Boris", "RK4" };
            if (ImGui::Combo("Integrator", &integratorType, integrators, 4)) {
                physicsEngine.setIntegrator(static_cast<physics::IntegratorFactory::Type>(integratorType));
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Statistics");
            ImGui::Separator();

            ImGui::Text("Sim Time: %.6e s", stats.simulationTime);
            ImGui::Text("Steps: %llu", stats.stepCount);
            ImGui::Text("Steps/sec: %.0f", stats.stepsPerSecond);
            ImGui::Text("Particles: %zu", stats.particleCount);
            ImGui::Text("Lost: %zu", stats.lostParticleCount);
            ImGui::Text("Avg Energy: %.3f GeV", stats.averageEnergy / (1e9 * physics::constants::energy::eV));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Rendering");
            ImGui::Separator();

            ImGui::Checkbox("Wireframe", &wireframeMode);

            if (ImGui::Checkbox("Show ImGui Demo", &showDemoWindow)) {}

            ImGui::Spacing();
            ImGui::Text("FPS: %.1f", 1.0 / deltaTime);

            ImGui::End();
        }

        // Demo window
        if (showDemoWindow) {
            ImGui::ShowDemoWindow(&showDemoWindow);
        }

        // Render ImGui
        ImGui::Render();

        // Clear and render scene
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        // Render accelerator and particles
        renderer.render(camera, *accelerator, physicsEngine.getParticleSystem());

        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // Render ImGui on top
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        window.swapBuffers();
    }

    PAS_INFO("Shutting down...");

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    utils::Logger::shutdown();

    return 0;
}
