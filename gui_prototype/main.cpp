// Museq GUI Prototype
// Requires GLFW and Dear ImGui source files in third_party/imgui

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

// Include Museq Engine
#include "AudioRenderer.h"
#include "ScriptParser.h"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s
", error, description);
}

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR autor, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Museq Studio", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Museq State
    char script_buffer[4096] = "// Write your Museq script here

instrument Piano {
    waveform sine
}

sequential {
    Piano { notes C4, E4, G4 }
}";
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- GUI LAYOUT ---
        
        // 1. Sidebar (Instruments)
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(250, io.DisplaySize.y));
        ImGui::Begin("Instruments", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        if (ImGui::Button("Add New Synth")) { /* logic */ }
        ImGui::Separator();
        ImGui::Text("Active Templates:");
        ImGui::BulletText("Piano (Lead)");
        ImGui::BulletText("SynthKick (Drum)");
        ImGui::End();

        // 2. Main Editor
        ImGui::SetNextWindowPos(ImVec2(250, 0));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 250, io.DisplaySize.y - 200));
        ImGui::Begin("Script Editor", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        if (ImGui::Button("Render & Play")) {
            // Logic to call ScriptParser and AudioRenderer
        }
        ImGui::InputTextMultiline("##editor", script_buffer, IM_ARRAYSIZE(script_buffer), ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_AllowTabInput);
        ImGui::End();

        // 3. Bottom Panel (Visualizer)
        ImGui::SetNextWindowPos(ImVec2(250, io.DisplaySize.y - 200));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 250, 200));
        ImGui::Begin("Master Output", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Waveform Preview:");
        // Simple placeholder for waveform visualization
        static float values[90] = { 0 };
        static int values_offset = 0;
        ImGui::PlotLines("##Waveform", values, IM_ARRAYSIZE(values), values_offset, NULL, -1.0f, 1.0f, ImVec2(-FLT_MIN, 100));
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
