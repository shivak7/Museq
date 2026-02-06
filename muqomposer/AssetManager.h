#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

struct SF2Preset {
    int bank;
    int preset;
    std::string name;
};

struct SF2Info {
    std::string path;
    std::string filename;
    std::vector<SF2Preset> presets;
};

class AssetManager {
public:
    AssetManager();

    // Folder Management
    void add_watched_folder(const std::string& path);
    const std::vector<std::string>& get_watched_folders() const;
    
    // Asset Retrieval
    void refresh_assets();
    const std::vector<SF2Info>& get_soundfonts() const;
    const std::vector<std::string>& get_samples() const;

private:
    std::vector<std::string> m_watched_folders;
    std::vector<SF2Info> m_soundfonts;
    std::vector<std::string> m_samples;

    void scan_directory(const fs::path& path);
    void process_sf2(const fs::path& path);
};

#endif // ASSET_MANAGER_H
