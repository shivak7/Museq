#ifndef SCRIPT_PARSER_H
#define SCRIPT_PARSER_H

#include "Song.h"
#include <memory>
#include <map>
#include <sstream>

// Forward declaration of FunctionDefinition and CompositeElement
struct FunctionDefinition;
class CompositeElement;

class ScriptParser {
public:
    static Song parse(const std::string& file_path);

private:
    Song m_song;
    std::map<std::string, FunctionDefinition> m_functions;
    std::map<std::string, Instrument> m_templates;
    
    // Default context for notes
    int m_default_duration = 500; // 120 BPM
    int m_default_velocity = 100;
    int m_default_octave = 4;

    ScriptParser();

    void collect_definitions(std::istream& input_stream);
    void process_script_stream(std::istream& input_stream, const std::map<std::string, std::string>& current_param_map, std::shared_ptr<CompositeElement> current_parent, int depth = 0);
    bool skipping_definition(const std::string& line, bool& in_function, bool& in_instrument, int& brace_count, std::istream& stream, int depth);
    std::string substitute_params(const std::string& line, const std::map<std::string, std::string>& param_map);
    void parse_compact_notes(const std::string& list, Sequence& seq);
};

// FunctionDefinition struct to hold function details
struct FunctionDefinition {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::string> body_lines;
};

#endif // SCRIPT_PARSER_H
