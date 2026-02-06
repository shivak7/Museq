#include "AssetManager.h"
#include "tsf.h"
#include <iostream>
#include <algorithm>

AssetManager::AssetManager() {
    // Default watched folder
    if (fs::exists("../sounds")) {
        add_watched_folder("../sounds");
    }
    if (fs::exists("sounds")) {
        add_watched_folder("sounds");
    }
}

void AssetManager::add_watched_folder(const std::string& path) {
    // Avoid duplicates
    if (std::find(m_watched_folders.begin(), m_watched_folders.end(), path) == m_watched_folders.end()) {
        if (fs::exists(path) && fs::is_directory(path)) {
            m_watched_folders.push_back(path);
            refresh_assets(); // Auto-refresh on add? Maybe.
        }
    }
}

const std::vector<std::string>& AssetManager::get_watched_folders() const {
    return m_watched_folders;
}

void AssetManager::refresh_assets() {
    m_soundfonts.clear();
    m_samples.clear();

    for (const auto& folder : m_watched_folders) {
        scan_directory(folder);
    }
}

void AssetManager::scan_directory(const fs::path& path) {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                // Lowercase extension for check
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".sf2") {
                    process_sf2(entry.path());
                } else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") {
                    m_samples.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directory " << path << ": " << e.what() << std::endl;
    }
}

void AssetManager::process_sf2(const fs::path& path) {
    SF2Info info;
    info.path = path.string();
    info.filename = path.filename().string();

    tsf* f = tsf_load_filename(path.string().c_str());
    if (f) {
        int count = tsf_get_presetcount(f);
        for (int i = 0; i < count; ++i) {
            info.presets.push_back({
                tsf_get_presetbank(f, i), 
                tsf_get_presetnumber(f, i), 
                std::string(tsf_get_presetname(f, i))
            });
        }
        tsf_close(f);
        m_soundfonts.push_back(info);
    }
}

const std::vector<SF2Info>& AssetManager::get_soundfonts() const {
    return m_soundfonts;
}

const std::vector<std::string>& AssetManager::get_samples() const {
    return m_samples;
}
