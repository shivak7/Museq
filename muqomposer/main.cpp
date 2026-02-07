// Museq GUI Prototype
// Requires GLFW and Dear ImGui source files in third_party/imgui

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

// Support for legacy OpenGL headers on Windows
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// Include Museq Engine
#include "AudioRenderer.h"
#include "AudioPlayer.h"
#include "ScriptParser.h"
#include "AssetManager.h"
#include "SplashUtils.h"
#include "WavWriter.h"
#include "Mp3Writer.h"
#include "OggWriter.h"
#include <fstream>
#include <filesystem>
#include <functional>
#include <algorithm>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

namespace fs = std::filesystem;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// Helper to load texture
bool load_texture_from_file(const char* filename, GLuint* out_texture, int* out_width, int* out_height, bool invert = false) {
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL) return false;

    // Optional: Invert colors (RGB only, keep alpha)
    if (invert) {
        for (int i = 0; i < image_width * image_height * 4; i += 4) {
            image_data[i]     = 255 - image_data[i];     // R
            image_data[i + 1] = 255 - image_data[i + 1]; // G
            image_data[i + 2] = 255 - image_data[i + 2]; // B
            // Alpha stays the same
        }
    }

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

void set_window_icon(GLFWwindow* window, const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (data) {
        GLFWimage icon;
        icon.width = width;
        icon.height = height;
        icon.pixels = data;
        glfwSetWindowIcon(window, 1, &icon);
        stbi_image_free(data);
    }
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

// Callback for Auto-Indent
static int editor_callback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        // We can handle specific character filtering here if needed
    }
    else if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways) {
        // Check for Enter key press manually? No, ImGui doesn't expose key events here easily.
        // We should use CallbackCompletion or custom logic outside if we want to intercept 'Enter' specifically.
        // Actually, the best way for auto-indent in ImGui is to check for \n insertion.
    }
    return 0;
}

// Helper to prepend code to the start of the buffer
void prepend_to_buffer(char* buffer, size_t buffer_size, const char* code) {
    size_t code_len = strlen(code);
    size_t current_len = strlen(buffer);
    if (code_len + current_len < buffer_size) {
        // Move current content to make room at the front
        memmove(buffer + code_len, buffer, current_len + 1);
        memcpy(buffer, code, code_len);
    }
}

// Improved Auto-Indent using a helper
void handle_auto_indent(char* buffer, size_t buffer_size) {
    static int last_len = 0;
    int current_len = (int)strlen(buffer);
    if (current_len == last_len + 1) {
        // Check if last character added was newline
        if (buffer[current_len - 1] == '\n') {
            // Find start of previous line
            int prev_line_start = current_len - 2;
            while (prev_line_start > 0 && buffer[prev_line_start - 1] != '\n') prev_line_start--;
            
            // Count leading spaces/tabs on previous line
            std::string indent = "";
            int i = prev_line_start;
            while (i < current_len - 1 && (buffer[i] == ' ' || buffer[i] == '\t')) {
                indent += buffer[i];
                i++;
            }
            
            // Append indent to buffer if it fits
            if (!indent.empty() && current_len + indent.length() < buffer_size) {
                strcat(buffer, indent.c_str());
            }
        }
    }
    last_len = (int)strlen(buffer);
}

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    // --- Phase 1: Borderless Initial State ---
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Start hidden to prevent flash

    // Get monitor resolution for scaling
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    int monitor_w = mode->width;
    int monitor_h = mode->height;

    // Create window with initial splash size
    // (We'll resize it once we load the image and know its aspect ratio)
    GLFWwindow* window = glfwCreateWindow(800, 800, "Muqomposer", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Robust path to logo
    std::string logo_path = "Museq_logo.png";
#ifdef MUSEQ_RESOURCE_DIR
    logo_path = std::string(MUSEQ_RESOURCE_DIR) + "/Museq_logo.png";
#endif

    // Set Window Icon
    set_window_icon(window, logo_path.c_str());

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- Splash Screen ---
    GLuint splash_texture = 0;
    int img_w = 0, img_h = 0;
    bool splash_loaded = load_texture_from_file(logo_path.c_str(), &splash_texture, &img_w, &img_h, true); // Invert colors

    if (splash_loaded) {
        // --- Phase 1: Dynamic Scaling and Centering ---
        SplashDimensions sd = calculate_splash_dims(monitor_w, monitor_h, img_w, img_h);
        glfwSetWindowSize(window, sd.width, sd.height);
        
        // Center window
        int xpos = (monitor_w - sd.width) / 2;
        int ypos = (monitor_h - sd.height) / 2;
        glfwSetWindowPos(window, xpos, ypos);
        
        glfwShowWindow(window);

        double start_time = glfwGetTime();
        while (glfwGetTime() - start_time < 2.0 && !glfwWindowShouldClose(window)) {
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)sd.width, (float)sd.height));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f)); // Dark background
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            
            ImGui::Begin("Splash", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs);
            
            // Draw scaled image to fill the top area (excluding 100px padding)
            float alpha = 1.0f;
            double elapsed = glfwGetTime() - start_time;
            if (elapsed < 0.5) alpha = (float)(elapsed / 0.5);
            if (elapsed > 1.5) alpha = (float)(1.0 - (elapsed - 1.5) / 0.5);
            
            // Image area is window size minus 100px from bottom
            float img_draw_h = (float)sd.height - 100.0f;
            ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)splash_texture, ImVec2(0, 0), ImVec2((float)sd.width, img_draw_h), ImVec2(0,0), ImVec2(1,1), ImColor(1.0f, 1.0f, 1.0f, alpha));
            
            // Draw Copyright Text at bottom
            ImGui::SetWindowFontScale(1.8f); // Slightly smaller font
            std::string copyright = "© 2026 Shiva Ratna (https://shivak7.github.io/)";
            float text_width = ImGui::CalcTextSize(copyright.c_str()).x; // Font scale applies to current window so CalcTextSize should respect it if inside window scope? 
            // Actually CalcTextSize uses GImGui->Font->FontSize * window_scale if called inside window.
            
            // Centering: (WindowWidth - TextWidth) / 2
            // Vertical: Start of padding + (PaddingHeight - TextHeight) / 2
            float text_x = ((float)sd.width - text_width) * 0.5f;
            float text_y = img_draw_h + (100.0f - ImGui::GetTextLineHeight()) * 0.5f;
            
            ImGui::SetCursorPos(ImVec2(text_x, text_y));

            // Fiery Glow Animation
            double time = glfwGetTime();
            float t1 = (float)((sin(time * 5.0) + 1.0) * 0.5); // 0..1
            float t2 = (float)((sin(time * 3.0 + 1.5) + 1.0) * 0.5); // 0..1
            
            ImVec4 col_red(0.8f, 0.1f, 0.0f, 1.0f);
            ImVec4 col_gold(1.0f, 0.8f, 0.0f, 1.0f);
            ImVec4 col_orange(1.0f, 0.5f, 0.0f, 1.0f);
            
            ImVec4 final_col;
            // Interpolate Red -> Gold
            final_col.x = col_red.x + (col_gold.x - col_red.x) * t1;
            final_col.y = col_red.y + (col_gold.y - col_red.y) * t1;
            final_col.z = col_red.z + (col_gold.z - col_red.z) * t1;
            
            // Mix with Orange
            final_col.x = final_col.x * (1.0f - t2) + col_orange.x * t2;
            final_col.y = final_col.y * (1.0f - t2) + col_orange.y * t2;
            final_col.z = final_col.z * (1.0f - t2) + col_orange.z * t2;
            
            final_col.w = alpha; // Apply global fade

            ImGui::TextColored(final_col, "%s", copyright.c_str());
            ImGui::SetWindowFontScale(1.0f); // Reset

            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }
        // Cleanup Splash Texture
        glDeleteTextures(1, &splash_texture);
    }

    // Restore Standard Window State after Splash
    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
    glfwSetWindowSize(window, 1280, 720);
    // Re-center restored window
    glfwSetWindowPos(window, (monitor_w - 1280) / 2, (monitor_h - 720) / 2);
    // In case splash failed to show it
    glfwShowWindow(window);

    // Museq Engine State
    AudioPlayer player;
    bool player_initialized = player.init();
    char status_text[128] = "Status: Ready";

    // Museq State
    char script_buffer[16384] = "// Write your Museq script here\n\ninstrument Piano {\n    waveform sine\n}\n\nsequential {\n    Piano { notes C4, E4, G4 }\n}";
    
    // Helper for Play logic
    auto play_logic = [&]() {
        if (player_initialized) {
            Song song = ScriptParser::parse_string(script_buffer);
            player.play(song);
        }
    };

    // Helper for Audio Preview
    auto play_preview = [&](const std::string& path, int bank = -1, int preset = -1) {
        if (!player_initialized) return;
        
        Song preview_song;
        Instrument preview_inst;
        if (bank >= 0) {
            preview_inst = Instrument("Preview", path, bank, preset);
        } else {
            // Check if it's a sample
            std::string ext = fs::path(path).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".sf2") {
                preview_inst = Instrument("Preview", path, 0, 0);
            } else {
                preview_inst = Instrument("Preview", path);
            }
        }
        
        preview_inst.sequence.add_note(Note(60, 1000, 100)); // C4 for 1s
        preview_song.root->children.push_back(std::make_shared<InstrumentElement>(preview_inst));
        
        player.play(preview_song, true);
    };

    // Helper for Export logic
    auto export_logic = [&](const std::string& path, int format) {
        Song song = ScriptParser::parse_string(script_buffer);
        AudioRenderer renderer;
        
        if (format == 0) {
            WavWriter writer;
            writer.write(renderer, song, path, 44100.0f);
        } else if (format == 1) {
            Mp3Writer writer;
            writer.write(renderer, song, path, 44100.0f);
        } else if (format == 2) {
            OggWriter writer;
            writer.write(renderer, song, path, 44100.0f);
        }
        
        snprintf(status_text, sizeof(status_text), "Status: Exported to %s", fs::path(path).filename().string().c_str());
    };

    // File State
    bool show_save_popup = false;
    bool show_load_popup = false;
    bool show_export_popup = false;
    char file_path_buffer[256] = "song.museq";
    char export_path_buffer[256] = "song.wav";
    int export_format = 0; // 0: WAV, 1: MP3, 2: OGG
    fs::path current_dir = fs::current_path();

    // Asset Browser State
    AssetManager asset_manager;
    asset_manager.refresh_assets();
    
    std::vector<std::string> active_instrument_names;

    auto update_active_instruments = [&]() {
        active_instrument_names.clear();
        std::stringstream ss(script_buffer);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find("instrument ") != std::string::npos) {
                size_t pos = line.find("instrument ") + 11;
                size_t end = line.find(" {", pos);
                if (end != std::string::npos) {
                    std::string name = line.substr(pos, end - pos);
                    // Trim leading/trailing whitespace from name
                    size_t first = name.find_first_not_of(" \t\r\n");
                    if (first != std::string::npos) {
                        size_t last = name.find_last_not_of(" \t\r\n");
                        name = name.substr(first, (last - first + 1));
                        active_instrument_names.push_back(name);
                    }
                }
            }
        }
    };

    update_active_instruments();

    // UI Layout Constants
    const float SIDEBAR_WIDTH = 250.0f;
    const float FOOTER_HEIGHT = 40.0f;
    
    // Mock Synths
    std::vector<std::string> synth_templates = {"SawLead", "SoftPad", "AcidBass", "Pluck"};

    // Helper for Asset Tree Rendering
    std::function<void(const AssetNode&)> render_asset_tree_node;
    render_asset_tree_node = [&](const AssetNode& node) {
        if (node.is_directory) {
            if (node.children.empty()) return; // Don't show empty folders? Or show them?
            if (ImGui::TreeNode(node.name.c_str())) {
                for (const auto& child : node.children) {
                    render_asset_tree_node(child);
                }
                ImGui::TreePop();
            }
        } else {
            // Favorite (Star) toggle
            ImGui::PushID(("fav" + node.full_path).c_str());
            bool fav = asset_manager.is_favorite(node.full_path);
            if (ImGui::Button(fav ? "[*]" : "[ ]", ImVec2(35, 0))) {
                asset_manager.toggle_favorite(node.full_path);
            }
            ImGui::PopID();
            ImGui::SameLine();

            // Audio Preview button for all leaf assets
            ImGui::PushID(("prev" + node.full_path).c_str());
            if (ImGui::Button("[>]", ImVec2(35, 0))) {
                play_preview(node.full_path);
            }
            ImGui::PopID();
            ImGui::SameLine();

            if (node.type == AssetType::SF2) {
                // Find presets for this SF2
                const auto& sfs = asset_manager.get_soundfonts();
                auto it = std::find_if(sfs.begin(), sfs.end(), [&](const SF2Info& i){ 
                    return fs::path(i.path) == fs::path(node.full_path); 
                });
                
                if (it != sfs.end()) {
                    if (ImGui::TreeNode(node.name.c_str())) {
                        for (const auto& p : it->presets) {
                            ImGui::PushID((node.full_path + p.name).c_str());
                            if (ImGui::Button("[>]", ImVec2(35, 0))) {
                                play_preview(it->path, p.bank, p.preset);
                            }
                            ImGui::PopID();
                            ImGui::SameLine();

                            if (ImGui::Selectable(p.name.c_str())) {
                                // Smart Insertion Check
                                if (!AssetManager::check_asset_exists_in_script(script_buffer, "soundfont", it->path, p.bank, p.preset)) {
                                    std::string sanitized_name = p.name;
                                    std::replace(sanitized_name.begin(), sanitized_name.end(), ' ', '_');
                                    
                                    std::string unique_name = AssetManager::get_unique_instrument_name(sanitized_name, active_instrument_names);
                                    active_instrument_names.push_back(unique_name); // Optimistic update

                                    char buf[512];
                                    snprintf(buf, sizeof(buf), "instrument %s {\n    soundfont \"%s\"\n    bank %d\n    preset %d\n}\n\n", 
                                        unique_name.c_str(), it->path.c_str(), p.bank, p.preset);
                                    prepend_to_buffer(script_buffer, IM_ARRAYSIZE(script_buffer), buf);
                                }
                            }
                            
                            // Drag and Drop Source
                            if (ImGui::BeginDragDropSource()) {
                                std::string sanitized_name = p.name;
                                std::replace(sanitized_name.begin(), sanitized_name.end(), ' ', '_');
                                
                                char buf[512];
                                snprintf(buf, sizeof(buf), "\ninstrument %s {\n    soundfont \"%s\"\n    bank %d\n    preset %d\n}", 
                                    sanitized_name.c_str(), it->path.c_str(), p.bank, p.preset);
                                ImGui::SetDragDropPayload("ASSET_CODE", buf, strlen(buf) + 1);
                                ImGui::Text("Add %s", p.name.c_str());
                                ImGui::EndDragDropSource();
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            } else if (node.type == AssetType::SAMPLE) {
                if (ImGui::Selectable(node.name.c_str())) {
                    // Smart Insertion Check
                    if (!AssetManager::check_asset_exists_in_script(script_buffer, "sample", node.full_path)) {
                        std::string clean_name = node.name.substr(0, node.name.find_last_of("."));
                        std::replace(clean_name.begin(), clean_name.end(), ' ', '_');
                        
                        std::string unique_name = AssetManager::get_unique_instrument_name(clean_name, active_instrument_names);
                        active_instrument_names.push_back(unique_name); // Optimistic update

                        char buf[512];
                        snprintf(buf, sizeof(buf), "instrument %s {\n    sample \"%s\"\n}\n\n", unique_name.c_str(), node.full_path.c_str());
                        prepend_to_buffer(script_buffer, IM_ARRAYSIZE(script_buffer), buf);
                    }
                }
                
                // Drag and Drop Source
                if (ImGui::BeginDragDropSource()) {
                    std::string clean_name = node.name.substr(0, node.name.find_last_of("."));
                    std::replace(clean_name.begin(), clean_name.end(), ' ', '_');
                    
                    char buf[512];
                    snprintf(buf, sizeof(buf), "\ninstrument %s {\n    sample \"%s\"\n}", clean_name.c_str(), node.full_path.c_str());
                    ImGui::SetDragDropPayload("ASSET_CODE", buf, strlen(buf) + 1);
                    ImGui::Text("Add %s", node.name.c_str());
                    ImGui::EndDragDropSource();
                }
            }
        }
    };

    // Helper for File Browser Logic
    auto render_file_browser = [&](const char* label) {
        ImGui::Text("Location: %s", current_dir.string().c_str());
        ImGui::BeginChild(label, ImVec2(450, 250), true);
        try {
            // Parent Directory Navigation
            if (current_dir.has_parent_path()) {
                if (ImGui::Selectable("[ .. ] Parent Directory")) {
                    current_dir = current_dir.parent_path();
                }
            }

            // Iterate Directory
            for (const auto& entry : fs::directory_iterator(current_dir)) {
                std::string filename = entry.path().filename().string();
                if (entry.is_directory()) {
                    if (ImGui::Selectable(("[DIR] " + filename).c_str())) {
                        current_dir /= filename;
                        break; // Exit loop to avoid iterator invalidation
                    }
                } else if (entry.is_regular_file() && entry.path().extension() == ".museq") {
                    if (ImGui::Selectable(filename.c_str())) {
                        strncpy(file_path_buffer, filename.c_str(), sizeof(file_path_buffer));
                    }
                }
            }
        } catch (const std::exception& e) {
            ImGui::TextColored(ImVec4(1,0,0,1), "Error: %s", e.what());
        }
        ImGui::EndChild();
    };

    // Folder Picker State
    bool show_folder_picker = false;
    
    // Search State
    char asset_search_buffer[128] = "";

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Keyboard Shortcuts
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_P)) play_logic();
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) show_save_popup = true;
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_L)) show_load_popup = true;
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Space)) player.stop();
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_E)) show_export_popup = true;

        // Slow updates
        static double last_parse_time = 0;
        if (ImGui::GetTime() - last_parse_time > 2.0) {
            update_active_instruments();
            last_parse_time = ImGui::GetTime();
        }

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
        
        ImGui::InputText("Search", asset_search_buffer, IM_ARRAYSIZE(asset_search_buffer));

        // Favorites Category
        if (ImGui::CollapsingHeader("Favorites", ImGuiTreeNodeFlags_DefaultOpen)) {
            const auto& favs = asset_manager.get_favorites();
            if (favs.empty()) ImGui::TextDisabled("No favorites yet. Click [*] to add.");
            for (const auto& f_path : favs) {
                std::string filename = fs::path(f_path).filename().string();
                
                ImGui::PushID(("fav_cat" + f_path).c_str());
                if (ImGui::Button("[*]", ImVec2(30, 0))) {
                    asset_manager.toggle_favorite(f_path);
                }
                ImGui::PopID();
                ImGui::SameLine();

                ImGui::PushID(("fav_cat_play" + f_path).c_str());
                if (ImGui::Button(">", ImVec2(20, 0))) {
                    play_preview(f_path);
                }
                ImGui::PopID();
                ImGui::SameLine();

                if (ImGui::Selectable(filename.c_str())) {
                    char buf[512];
                    std::string ext = fs::path(f_path).extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".sf2") {
                        snprintf(buf, sizeof(buf), "instrument %s {\n    soundfont \"%s\"\n    bank 0\n    preset 0\n}\n\n", filename.c_str(), f_path.c_str());
                    } else {
                        snprintf(buf, sizeof(buf), "instrument %s {\n    sample \"%s\"\n}\n\n", filename.c_str(), f_path.c_str());
                    }
                    prepend_to_buffer(script_buffer, IM_ARRAYSIZE(script_buffer), buf);
                }
            }
        }

        if (ImGui::Button("Add New Synth", ImVec2(-FLT_MIN, 0))) { 
            std::string new_synth = "instrument NewSynth {\n    waveform sawtooth\n    envelope 0.01 0.1 0.8 0.2\n    filter lowpass 2000 1.0\n}\n\n";
            prepend_to_buffer(script_buffer, IM_ARRAYSIZE(script_buffer), new_synth.c_str());
        }
        
        // Add Folder Button
        if (ImGui::Button("Add Folder", ImVec2(-FLT_MIN, 0))) {
            show_folder_picker = true;
        }

        if (ImGui::Button("Clear All", ImVec2(-FLT_MIN, 0))) { asset_manager.clear_watched_folders(); }
        if (ImGui::Button("Refresh Assets", ImVec2(-FLT_MIN, 0))) { asset_manager.refresh_assets(); }
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Active / Imported", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (active_instrument_names.empty()) ImGui::TextDisabled("No instruments defined");
            for (const auto& inst : active_instrument_names) {
                if (strlen(asset_search_buffer) == 0 || inst.find(asset_search_buffer) != std::string::npos) {
                    ImGui::BulletText("%s", inst.c_str());
                }
            }
        }

        if (ImGui::CollapsingHeader("SoundFonts")) {
            AssetNode sf_tree = asset_manager.get_soundfont_tree(asset_search_buffer);
            for (const auto& child : sf_tree.children) {
                render_asset_tree_node(child);
            }
        }

        if (ImGui::CollapsingHeader("Samples")) {
            AssetNode smp_tree = asset_manager.get_sample_tree(asset_search_buffer);
            for (const auto& child : smp_tree.children) {
                render_asset_tree_node(child);
            }
        }

        if (ImGui::CollapsingHeader("Synths")) {
            for (const auto& synth : synth_templates) {
                ImGui::PushID(synth.c_str());
                if (ImGui::Button("[>]", ImVec2(35, 0))) {
                    // Preview Synth
                    Song preview_song;
                    Instrument preview_inst(synth, Waveform::SAWTOOTH); // Default for template
                    preview_inst.sequence.add_note(Note(60, 1000, 100));
                    preview_song.root->children.push_back(std::make_shared<InstrumentElement>(preview_inst));
                    player.play(preview_song, true);
                }
                ImGui::PopID();
                ImGui::SameLine();

                if (ImGui::Selectable(synth.c_str())) {
                    std::string sanitized_name = synth;
                    std::replace(sanitized_name.begin(), sanitized_name.end(), ' ', '_');
                    
                    std::string unique_name = AssetManager::get_unique_instrument_name(sanitized_name, active_instrument_names);
                    active_instrument_names.push_back(unique_name);

                    char buf[512];
                    snprintf(buf, sizeof(buf), "instrument %s {\n    waveform sawtooth\n    envelope 0.1 0.2 0.5 0.3\n}\n\n", unique_name.c_str());
                    prepend_to_buffer(script_buffer, IM_ARRAYSIZE(script_buffer), buf);
                }
            }
        }
        ImGui::End();

        // --- Folder Picker Popup ---
        if (show_folder_picker) {
            ImGui::OpenPopup("Select Folder");
        }

        if (ImGui::BeginPopupModal("Select Folder", &show_folder_picker, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Current: %s", current_dir.string().c_str());
            
            ImGui::BeginChild("DirBrowser", ImVec2(400, 200), true);
            if (current_dir.has_parent_path()) {
                if (ImGui::Selectable("[ .. ] Up")) current_dir = current_dir.parent_path();
            }
            try {
                for (const auto& entry : fs::directory_iterator(current_dir)) {
                    if (entry.is_directory()) {
                        if (ImGui::Selectable(entry.path().filename().string().c_str())) {
                            current_dir /= entry.path().filename();
                            break;
                        }
                    }
                }
            } catch(...) {}
            ImGui::EndChild();

            if (ImGui::Button("Select This Folder", ImVec2(150, 0))) {
                asset_manager.add_watched_folder(current_dir.string());
                show_folder_picker = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                show_folder_picker = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

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

        // --- Editor with Line Numbers ---
        float line_number_width = 40.0f;
        float editor_width = ImGui::GetContentRegionAvail().x - line_number_width;

        // Synchronize scrolling
        static float scroll_y = 0.0f;

        // Line Numbers column
        ImGui::BeginChild("LineNumbers", ImVec2(line_number_width, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetScrollY(scroll_y);
        int line_count = 1;
        for (int i = 0; script_buffer[i]; i++) if (script_buffer[i] == '\n') line_count++;
        for (int i = 1; i <= line_count; i++) {
            ImGui::TextDisabled("%4d", i);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Editor column
        ImGui::BeginChild("EditorColumn", ImVec2(editor_width, 0));
        if (ImGui::InputTextMultiline("##editor", script_buffer, IM_ARRAYSIZE(script_buffer), ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_AllowTabInput)) {
            handle_auto_indent(script_buffer, IM_ARRAYSIZE(script_buffer));
        }
        scroll_y = ImGui::GetScrollY();
        ImGui::EndChild();
        
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_CODE")) {
                const char* code = (const char*)payload->Data;
                
                // Parse dropped code to check existence
                std::string code_str = code;
                std::string type, path;
                int bank = -1, preset = -1;
                bool should_add = true;

                if (code_str.find("soundfont \"") != std::string::npos) {
                    type = "soundfont";
                    size_t s = code_str.find("soundfont \"") + 11;
                    size_t e = code_str.find("\"", s);
                    if (s != std::string::npos && e != std::string::npos) path = code_str.substr(s, e-s);
                    
                    if (code_str.find("bank ") != std::string::npos) {
                        std::stringstream ss(code_str.substr(code_str.find("bank ")));
                        std::string tmp; ss >> tmp >> bank;
                    }
                    if (code_str.find("preset ") != std::string::npos) {
                        std::stringstream ss(code_str.substr(code_str.find("preset ")));
                        std::string tmp; ss >> tmp >> preset;
                    }
                } else if (code_str.find("sample \"") != std::string::npos) {
                    type = "sample";
                    size_t s = code_str.find("sample \"") + 8;
                    size_t e = code_str.find("\"", s);
                    if (s != std::string::npos && e != std::string::npos) path = code_str.substr(s, e-s);
                }

                if (!type.empty()) {
                    if (AssetManager::check_asset_exists_in_script(script_buffer, type, path, bank, preset)) {
                        should_add = false;
                    }
                }

                if (should_add) {
                    std::string final_code = code;
                    
                    // Extract name and handle conflict
                    size_t instr_pos = final_code.find("instrument ");
                    if (instr_pos != std::string::npos) {
                        size_t name_start = instr_pos + 11;
                        size_t name_end = final_code.find(" {", name_start);
                        if (name_end != std::string::npos) {
                            std::string base_name = final_code.substr(name_start, name_end - name_start);
                            std::string unique_name = AssetManager::get_unique_instrument_name(base_name, active_instrument_names);
                            
                            if (unique_name != base_name) {
                                final_code.replace(name_start, name_end - name_start, unique_name);
                            }
                            active_instrument_names.push_back(unique_name);
                        }
                    }

                    prepend_to_buffer(script_buffer, IM_ARRAYSIZE(script_buffer), final_code.c_str());
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::End();

        // --- 3. VISUALIZER (Bottom Right) ---
        ImGui::SetNextWindowPos(ImVec2(SIDEBAR_WIDTH, editor_height));
        ImGui::SetNextWindowSize(ImVec2(main_area_width, visualizer_height));
        ImGui::Begin("Visualizer", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        float vis_avail_w = ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("WaveformView", ImVec2(vis_avail_w * 0.5f, 0));
        ImGui::TextDisabled("Output Waveform");
        static float vis_samples[512];
        player.get_visualization_data(vis_samples, 512);
        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
        ImGui::PlotLines("##Waveform", vis_samples, IM_ARRAYSIZE(vis_samples), 0, NULL, -1.0f, 1.0f, ImVec2(-FLT_MIN, -FLT_MIN));
        ImGui::PopStyleColor();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("SpectrumView", ImVec2(0, 0));
        ImGui::TextDisabled("Spectrum Analyzer");
        static float spec_samples[256];
        player.get_spectrum_data(spec_samples, 256);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
        ImGui::PlotHistogram("##Spectrum", spec_samples, IM_ARRAYSIZE(spec_samples), 0, NULL, 0.0f, 0.05f, ImVec2(-FLT_MIN, -FLT_MIN));
        ImGui::PopStyleColor();
        ImGui::EndChild();

        ImGui::End();

        // --- 4. FOOTER (Transport Toolbar) ---
        ImGui::SetNextWindowPos(ImVec2(0, main_area_height));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, FOOTER_HEIGHT));
        ImGui::Begin("Footer", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
        
        // Group Left: Status
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", status_text);
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        // Group Center: Transport
        if (ImGui::Button("[ > ] Play", ImVec2(80, 0))) {
            play_logic();
        }
        ImGui::SameLine();
        if (ImGui::Button("[ || ] Stop", ImVec2(80, 0))) {
            player.stop();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        // Group BPM
        static int bpm = 120;
        ImGui::SetNextItemWidth(120);
        if (ImGui::SliderInt("BPM", &bpm, 40, 240)) {
            ScriptParser::set_global_bpm(bpm);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        // Group Right: Files
        float right_btns_w = 260.0f;
        avail_w = ImGui::GetContentRegionAvail().x;
        if (avail_w > right_btns_w) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail_w - right_btns_w);
        }

        if (ImGui::Button("Load", ImVec2(80, 0))) {
            show_load_popup = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(80, 0))) {
            show_save_popup = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Export", ImVec2(80, 0))) {
            show_export_popup = true;
        }
        ImGui::End();

        // --- Save Popup ---
        if (show_save_popup) {
            ImGui::OpenPopup("Save Script");
        }

        if (ImGui::BeginPopupModal("Save Script", &show_save_popup, ImGuiWindowFlags_AlwaysAutoResize)) {
            render_file_browser("SaveBrowser");

            ImGui::Text("Save as:");
            ImGui::InputText("##filename", file_path_buffer, IM_ARRAYSIZE(file_path_buffer));
            
            if (ImGui::Button("Save", ImVec2(120, 0))) {
                fs::path full_path = current_dir / file_path_buffer;
                save_script_to_file(full_path.string().c_str(), script_buffer);
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
            render_file_browser("LoadBrowser");

            ImGui::Text("Selected:");
            ImGui::InputText("##filename", file_path_buffer, IM_ARRAYSIZE(file_path_buffer));
            
            if (ImGui::Button("Load", ImVec2(120, 0))) {
                fs::path full_path = current_dir / file_path_buffer;
                if (load_script_from_file(full_path.string().c_str(), script_buffer, IM_ARRAYSIZE(script_buffer))) {
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

        // --- Export Popup ---
        if (show_export_popup) {
            ImGui::OpenPopup("Export Song");
        }

        if (ImGui::BeginPopupModal("Export Song", &show_export_popup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Export Settings");
            ImGui::Separator();

            const char* formats[] = { "WAV (Lossless)", "MP3 (Compressed)", "OGG (Vorbis)" };
            if (ImGui::Combo("Format", &export_format, formats, IM_ARRAYSIZE(formats))) {
                // Update extension
                std::string base = fs::path(export_path_buffer).stem().string();
                if (export_format == 0) snprintf(export_path_buffer, sizeof(export_path_buffer), "%s.wav", base.c_str());
                else if (export_format == 1) snprintf(export_path_buffer, sizeof(export_path_buffer), "%s.mp3", base.c_str());
                else if (export_format == 2) snprintf(export_path_buffer, sizeof(export_path_buffer), "%s.ogg", base.c_str());
            }

            ImGui::Text("Filename:");
            ImGui::InputText("##exportfilename", export_path_buffer, IM_ARRAYSIZE(export_path_buffer));

            if (ImGui::Button("Export", ImVec2(120, 0))) {
                fs::path full_path = current_dir / export_path_buffer;
                export_logic(full_path.string(), export_format);
                show_export_popup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                show_export_popup = false;
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
