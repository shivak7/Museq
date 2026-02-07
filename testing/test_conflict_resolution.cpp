#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>

// Mocking the helper
std::string get_unique_name(const std::vector<std::string>& existing_names, const std::string& base_name) {
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

int main() {
    std::vector<std::string> names = {"Piano", "Drums", "Bass"};
    
    // Case 1: No conflict
    assert(get_unique_name(names, "Violin") == "Violin");
    
    // Case 2: Conflict
    assert(get_unique_name(names, "Piano") == "Piano_1");
    
    // Case 3: Multiple conflicts
    names.push_back("Piano_1");
    assert(get_unique_name(names, "Piano") == "Piano_2");
    
    // Case 4: Gap
    names.push_back("Piano_3"); // Piano_2 is missing
    assert(get_unique_name(names, "Piano") == "Piano_2");

    std::cout << "Conflict resolution logic passed!" << std::endl;
    return 0;
}
