#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

enum class AssetType {
    NONE,
    SF2,
    SAMPLE,
    DIRECTORY
};

struct AssetNode {
    std::string name;
    std::string full_path;
    AssetType type;
    std::vector<AssetNode> children;
    bool is_directory; // Helper for test/ui
};

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
    void clear_watched_folders();
    const std::vector<std::string>& get_watched_folders() const;

    // Helper to check for existing definitions in script
    static bool check_asset_exists_in_script(const std::string& script, const std::string& type, const std::string& path, int bank = -1, int preset = -1);
    
    // Helper to resolve name conflicts
    static std::string get_unique_instrument_name(const std::string& base_name, const std::vector<std::string>& existing_names);

    // Asset Retrieval
    void refresh_assets();
    const std::vector<SF2Info>& get_soundfonts() const;
    const std::vector<std::string>& get_samples() const;

    // Tree Views
    AssetNode get_soundfont_tree(const std::string& filter = "") const;
    AssetNode get_sample_tree(const std::string& filter = "") const;

    // Favorites
    void toggle_favorite(const std::string& path);
    bool is_favorite(const std::string& path) const;
    const std::vector<std::string>& get_favorites() const;
    void load_favorites();
    void save_favorites();

private:
    std::vector<std::string> m_watched_folders;
    std::vector<SF2Info> m_soundfonts;
    std::vector<std::string> m_samples;
    std::vector<std::string> m_favorites;

    void scan_directory(const fs::path& path);
    void process_sf2(const fs::path& path);
    
    // Helper to build tree from flat list
    AssetNode build_tree_from_paths(const std::vector<std::string>& paths, AssetType leaf_type, const std::string& filter) const;
};

#endif // ASSET_MANAGER_H