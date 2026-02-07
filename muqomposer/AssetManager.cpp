#include "AssetManager.h"
#include "tsf.h"
#include <iostream>
#include <algorithm>
#include <fstream>

AssetManager::AssetManager() {
    // Default watched folder
    if (fs::exists("../sounds")) {
        add_watched_folder(fs::absolute("../sounds").string());
    }
    if (fs::exists("sounds")) {
        add_watched_folder(fs::absolute("sounds").string());
    }
    load_favorites();
}

bool AssetManager::check_asset_exists_in_script(const std::string& script, const std::string& type, const std::string& path, int bank, int preset) {
    auto normalize_path = [](std::string p) {
        std::replace(p.begin(), p.end(), '\\', '/');
        // Remove ./ prefix if any
        if (p.find("./") == 0) p = p.substr(2);
        return p;
    };

    std::stringstream ss(script);
    std::string line;
    bool inside_instrument = false;
    
    std::string current_inst_type;
    std::string current_inst_path;
    int current_inst_bank = -1;
    int current_inst_preset = -1;
    
    while (std::getline(ss, line)) {
        if (line.find("instrument ") != std::string::npos && line.find("{") != std::string::npos) {
            inside_instrument = true;
            current_inst_type = "";
            current_inst_path = "";
            current_inst_bank = -1;
            current_inst_preset = -1;
            continue;
        }
        
        if (inside_instrument) {
            if (line.find("}") != std::string::npos) {
                inside_instrument = false;
                // End of instrument, check match
                if (type == "soundfont") {
                    bool path_match = (normalize_path(current_inst_path) == normalize_path(path));
                    
                    if (current_inst_type == "soundfont" && 
                        path_match &&
                        current_inst_bank == bank &&
                        current_inst_preset == preset) {
                        return true;
                    }
                } else if (type == "sample") {
                    bool path_match = (normalize_path(current_inst_path) == normalize_path(path));
                    
                    if (current_inst_type == "sample" && path_match) {
                        return true;
                    }
                }
                continue;
            }
            
            // Parse fields
            if (line.find("soundfont ") != std::string::npos) {
                size_t start = line.find("\"");
                if (start != std::string::npos) {
                    start++;
                    size_t end = line.find("\"", start);
                    if (end != std::string::npos) {
                        current_inst_path = line.substr(start, end - start);
                        current_inst_type = "soundfont";
                    }
                }
            }
            else if (line.find("sample ") != std::string::npos) {
                size_t start = line.find("\"");
                if (start != std::string::npos) {
                    start++;
                    size_t end = line.find("\"", start);
                    if (end != std::string::npos) {
                        current_inst_path = line.substr(start, end - start);
                        current_inst_type = "sample";
                    }
                }
            }
            else if (line.find("bank ") != std::string::npos) {
                std::stringstream ls(line);
                std::string temp;
                int val;
                ls >> temp >> val; 
                current_inst_bank = val;
            }
            else if (line.find("preset ") != std::string::npos) {
                std::stringstream ls(line);
                std::string temp;
                int val;
                ls >> temp >> val;
                current_inst_preset = val;
            }
        }
    }
    return false;
}

std::string AssetManager::get_unique_instrument_name(const std::string& base_name, const std::vector<std::string>& existing_names) {
    if (std::find(existing_names.begin(), existing_names.end(), base_name) == existing_names.end()) {
        return base_name;
    }
    
    int index = 1;
    while (true) {
        std::string candidate = base_name + "_" + std::to_string(index);
        if (std::find(existing_names.begin(), existing_names.end(), candidate) == existing_names.end()) {
            return candidate;
        }
        index++;
    }
}

void AssetManager::toggle_favorite(const std::string& path) {
    auto it = std::find(m_favorites.begin(), m_favorites.end(), path);
    if (it != m_favorites.end()) {
        m_favorites.erase(it);
    } else {
        m_favorites.push_back(path);
    }
    save_favorites();
}

bool AssetManager::is_favorite(const std::string& path) const {
    return std::find(m_favorites.begin(), m_favorites.end(), path) != m_favorites.end();
}

const std::vector<std::string>& AssetManager::get_favorites() const {
    return m_favorites;
}

void AssetManager::load_favorites() {
    std::ifstream in("favorites.txt");
    if (in.is_open()) {
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty()) m_favorites.push_back(line);
        }
    }
}

void AssetManager::save_favorites() {
    std::ofstream out("favorites.txt");
    if (out.is_open()) {
        for (const auto& f : m_favorites) {
            out << f << "\n";
        }
    }
}

void AssetManager::add_watched_folder(const std::string& path) {
    if (fs::exists(path) && fs::is_directory(path)) {
        std::string abs_path = fs::absolute(path).string();
        // Avoid duplicates
        if (std::find(m_watched_folders.begin(), m_watched_folders.end(), abs_path) == m_watched_folders.end()) {
            m_watched_folders.push_back(abs_path);
            refresh_assets();
        }
    }
}

void AssetManager::clear_watched_folders() {
    m_watched_folders.clear();
    refresh_assets();
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
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".sf2") {
                    process_sf2(entry.path());
                } else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") {
                    m_samples.push_back(fs::absolute(entry.path()).string());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directory " << path << ": " << e.what() << std::endl;
    }
}

void AssetManager::process_sf2(const fs::path& path) {
    SF2Info info;
    info.path = fs::absolute(path).string();
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

// Tree Implementation

AssetNode AssetManager::get_sample_tree(const std::string& filter) const {
    return build_tree_from_paths(m_samples, AssetType::SAMPLE, filter);
}

AssetNode AssetManager::get_soundfont_tree(const std::string& filter) const {
    std::vector<std::string> paths;
    for (const auto& sf : m_soundfonts) {
        paths.push_back(sf.path);
    }
    return build_tree_from_paths(paths, AssetType::SF2, filter);
}

AssetNode AssetManager::build_tree_from_paths(const std::vector<std::string>& paths, AssetType leaf_type, const std::string& filter) const {
    AssetNode root;
    root.name = "Root";
    root.type = AssetType::DIRECTORY;
    root.is_directory = true;

    // Normalize filter to lowercase
    std::string filter_lower = filter;
    std::transform(filter_lower.begin(), filter_lower.end(), filter_lower.begin(), ::tolower);

    for (const auto& watched : m_watched_folders) {
        fs::path watched_path = fs::path(watched);
        
        // Create a node for the watched folder
        AssetNode watched_node;
        watched_node.name = watched_path.filename().string();
        if (watched_node.name.empty()) watched_node.name = watched; // fallback
        watched_node.full_path = watched;
        watched_node.type = AssetType::DIRECTORY;
        watched_node.is_directory = true;

        // Filter and add paths belonging to this folder
        bool has_children = false;
        
        for (const auto& p_str : paths) {
            // Apply filter
            if (!filter_lower.empty()) {
                fs::path p_tmp(p_str);
                std::string name_lower = p_tmp.filename().string();
                std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
                if (name_lower.find(filter_lower) == std::string::npos) {
                    continue; // Skip if filter not found in filename
                }
            }

            fs::path p = fs::path(p_str);
            
            // Check if p starts with watched_path
            if (p_str.find(watched) == 0) { // p starts with watched
                has_children = true;
                fs::path rel = fs::relative(p, watched_path);
                
                // Traverse/Build
                AssetNode* current = &watched_node;
                for (const auto& part : rel) {
                    std::string part_name = part.string();
                    bool is_last = (part == rel.filename());
                    
                    // Check if child exists
                    auto it = std::find_if(current->children.begin(), current->children.end(), 
                        [&](const AssetNode& n){ return n.name == part_name; });
                    
                    if (it != current->children.end()) {
                        current = &(*it);
                    } else {
                        AssetNode new_node;
                        new_node.name = part_name;
                        new_node.full_path = (fs::path(current->full_path) / part).string();
                        if (is_last) {
                            new_node.type = leaf_type;
                            new_node.is_directory = false;
                        } else {
                            new_node.type = AssetType::DIRECTORY;
                            new_node.is_directory = true;
                        }
                        current->children.push_back(new_node);
                        current = &current->children.back();
                    }
                }
            }
        }
        
        // Only add watched folder if it has matching assets
        if (has_children) {
            root.children.push_back(watched_node);
        }
    }
    
    return root;
}
