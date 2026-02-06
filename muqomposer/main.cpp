// Museq GUI Prototype
// Requires GLFW and Dear ImGui source files in third_party/imgui

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

// Include Museq Engine
#include "AudioRenderer.h"
#include "AudioPlayer.h"
#include "ScriptParser.h"
#include <fstream>

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void save_script_to_file(const char* filename, const char* content) {
    std::ofstream out(filename);
    if (out.is_open()) {
        out << content;
        out.close();
    } else {
        fprintf(stderr, "Failed to save file: %s\n", filename);
    }
}

bool load_script_from_file(const char* filename, char* buffer, size_t buffer_size) {
    std::ifstream in(filename);
    if (in.is_open()) {
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        if (content.length() < buffer_size) {
            strncpy(buffer, content.c_str(), buffer_size);
            buffer[buffer_size - 1] = '\0';
            return true;
        }
    }
    return false;
}

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Muqomposer", NULL, NULL);
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

    // Museq Engine State
    AudioPlayer player;
    bool player_initialized = player.init();
    char status_text[128] = "Status: Ready";

    // Museq State
    char script_buffer[16384] = "// Write your Museq script here\n\ninstrument Piano {\n    waveform sine\n}\n\nsequential {\n    Piano { notes C4, E4, G4 }\n}";
    
    // File State
    bool show_save_popup = false;
    bool show_load_popup = false;
    char file_path_buffer[256] = "song.museq";

    // UI Layout Constants
    const float SIDEBAR_WIDTH = 250.0f;
    const float FOOTER_HEIGHT = 40.0f;
    
    // Mock Data
    std::vector<std::string> active_instruments = {"Piano", "Drums"};
    std::vector<std::string> soundfonts = {"GeneralUser.sf2", "Arachno.sf2"};
    std::vector<std::string> samples = {"kick.wav", "snare.wav", "hat.wav"};
    std::vector<std::string> synths = {"SawLead", "Pad", "Bass"};

    // Helper for Play logic
    auto play_logic = [&]() {
        if (player_initialized) {
            Song song = ScriptParser::parse_string(script_buffer);
            player.play(song);
        }
    };

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Update Status
        if (!player_initialized) {
            snprintf(status_text, sizeof(status_text), "Status: Audio Init Failed");
        } else if (player.is_playing()) {
            snprintf(status_text, sizeof(status_text), "Status: Playing");
        } else {
            snprintf(status_text, sizeof(status_text), "Status: Ready");
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Calculate layout dimensions
        float main_area_width = io.DisplaySize.x - SIDEBAR_WIDTH;
        float main_area_height = io.DisplaySize.y - FOOTER_HEIGHT;
        float editor_height = main_area_height * 0.65f;
        float visualizer_height = main_area_height - editor_height;

        // --- 1. LEFT SIDEBAR (Asset Browser) ---
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(SIDEBAR_WIDTH, main_area_height));
        ImGui::Begin("Assets", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        ImGui::SeparatorText("ASSETS");

        if (ImGui::Button("Add New Synth", ImVec2(-FLT_MIN, 0))) { 
            std::string new_synth = "\n\ninstrument NewSynth {\n    waveform sawtooth\n    envelope 0.01 0.1 0.8 0.2\n    filter lowpass 2000 1.0\n}";
            if (strlen(script_buffer) + new_synth.length() < IM_ARRAYSIZE(script_buffer)) {
                strcat(script_buffer, new_synth.c_str());
            }
        }
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Active / Imported", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (const auto& inst : active_instruments) {
                ImGui::BulletText("%s", inst.c_str());
            }
        }

        if (ImGui::CollapsingHeader("SoundFonts")) {
            for (const auto& sf : soundfonts) {
                if (ImGui::TreeNode(sf.c_str())) {
                    ImGui::Text("  (Bank 0, Preset 0)");
                    ImGui::TreePop();
                }
            }
        }

        if (ImGui::CollapsingHeader("Samples")) {
            for (const auto& smp : samples) {
                ImGui::Selectable(smp.c_str());
            }
        }

        if (ImGui::CollapsingHeader("Synths")) {
            for (const auto& synth : synths) {
                ImGui::Selectable(synth.c_str());
            }
        }
        ImGui::End();

        // --- 2. CODE EDITOR (Top Right) ---
        ImGui::SetNextWindowPos(ImVec2(SIDEBAR_WIDTH, 0));
        ImGui::SetNextWindowSize(ImVec2(main_area_width, editor_height));
        ImGui::Begin("Code Interface", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        ImGui::TextDisabled("Code Interface");
        ImGui::SameLine();
        float avail_w = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail_w - 110);
        if (ImGui::Button("Render & Play")) {
            play_logic();
        }

        ImGui::InputTextMultiline("##editor", script_buffer, IM_ARRAYSIZE(script_buffer), ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_AllowTabInput);
        ImGui::End();

        // --- 3. VISUALIZER (Bottom Right) ---
        ImGui::SetNextWindowPos(ImVec2(SIDEBAR_WIDTH, editor_height));
        ImGui::SetNextWindowSize(ImVec2(main_area_width, visualizer_height));
        ImGui::Begin("Visualizer", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        ImGui::TextDisabled("Output Waveform");
        
        static float vis_samples[512];
        player.get_visualization_data(vis_samples, 512);

        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
        ImGui::PlotLines("##Waveform", vis_samples, IM_ARRAYSIZE(vis_samples), 0, NULL, -1.0f, 1.0f, ImVec2(-FLT_MIN, -FLT_MIN));
        ImGui::PopStyleColor();
        ImGui::End();

        // --- 4. FOOTER (Status Bar) ---
        ImGui::SetNextWindowPos(ImVec2(0, main_area_height));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, FOOTER_HEIGHT));
        ImGui::Begin("Footer", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
        
        // Status Text
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", status_text);
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        
        // Song Params
        ImGui::Text("BPM: 120  Sig: 4/4");
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        // Transport Controls
        float button_width = 60.0f;
        
        // Right align buttons roughly
        float available_width = ImGui::GetContentRegionAvail().x;
        float buttons_start = available_width - (button_width * 4 + 25);
        if (buttons_start > 0) ImGui::SameLine(buttons_start + ImGui::GetCursorPosX());

        if (ImGui::Button("Play", ImVec2(button_width, 0))) {
            play_logic();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop", ImVec2(button_width, 0))) {
            player.stop();
        }
        ImGui::SameLine();
        if (ImGui::Button("Load", ImVec2(button_width, 0))) {
            show_load_popup = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(button_width, 0))) {
            show_save_popup = true;
        }
        ImGui::End();

        // --- Save Popup ---
        if (show_save_popup) {
            ImGui::OpenPopup("Save Script");
        }

        if (ImGui::BeginPopupModal("Save Script", &show_save_popup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter filename:");
            ImGui::InputText("##filename", file_path_buffer, IM_ARRAYSIZE(file_path_buffer));
            
            if (ImGui::Button("Save", ImVec2(120, 0))) {
                save_script_to_file(file_path_buffer, script_buffer);
                show_save_popup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                show_save_popup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // --- Load Popup ---
        if (show_load_popup) {
            ImGui::OpenPopup("Load Script");
        }

        if (ImGui::BeginPopupModal("Load Script", &show_load_popup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter filename:");
            ImGui::InputText("##filename", file_path_buffer, IM_ARRAYSIZE(file_path_buffer));
            
            if (ImGui::Button("Load", ImVec2(120, 0))) {
                if (load_script_from_file(file_path_buffer, script_buffer, IM_ARRAYSIZE(script_buffer))) {
                    show_load_popup = false;
                    ImGui::CloseCurrentPopup();
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to load file!");
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                show_load_popup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

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
