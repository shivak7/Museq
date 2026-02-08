#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

struct Settings {
    float ui_font_size = 15.0f;
    float editor_font_size = 18.0f;

    static Settings load(const std::string& filename = "settings.json") {
        Settings s;
        if (fs::exists(filename)) {
            try {
                std::ifstream in(filename);
                json j;
                in >> j;
                if (j.contains("ui_font_size")) s.ui_font_size = j["ui_font_size"];
                if (j.contains("editor_font_size")) s.editor_font_size = j["editor_font_size"];
            } catch (...) {
                // Fallback to defaults on parse error
            }
        }
        return s;
    }

    void save(const std::string& filename = "settings.json") const {
        json j;
        j["ui_font_size"] = ui_font_size;
        j["editor_font_size"] = editor_font_size;
        std::ofstream out(filename);
        out << j.dump(4);
    }
};

#endif // SETTINGS_H
