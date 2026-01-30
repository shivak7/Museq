#include "ScriptParser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "NoteParser.h"
#include "SongElement.h"

ScriptParser::ScriptParser() {}

Song ScriptParser::parse(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open script file " << file_path << std::endl;
        return Song();
    }

    ScriptParser parser;
    parser.collect_definitions(file);
    file.clear();
    file.seekg(0, std::ios::beg);
    
    std::map<std::string, std::string> empty_params;
    parser.process_script_stream(file, empty_params, parser.m_song.root);

    return parser.m_song;
}

void ScriptParser::collect_definitions(std::istream& input_stream) {
    std::string line;
    while (std::getline(input_stream, line)) {
        std::stringstream ss(line);
        std::string keyword;
        if (!(ss >> keyword)) continue;

        if (keyword == "function") {
            FunctionDefinition func;
            std::string name_and_params;
            std::getline(ss, name_and_params, '{');
            
            size_t open_paren = name_and_params.find('(');
            size_t close_paren = name_and_params.find(')');
            func.name = name_and_params.substr(0, open_paren);
            func.name.erase(std::remove_if(func.name.begin(), func.name.end(), ::isspace), func.name.end());

            if (open_paren != std::string::npos && close_paren != std::string::npos) {
                std::string params_str = name_and_params.substr(open_paren + 1, close_paren - open_paren - 1);
                std::stringstream pss(params_str);
                std::string param;
                while (std::getline(pss, param, ',')) {
                    param.erase(std::remove_if(param.begin(), param.end(), ::isspace), param.end());
                    func.parameters.push_back(param);
                }
            }

            int brace_count = 1;
            while (std::getline(input_stream, line) && brace_count > 0) {
                if (line.find('{') != std::string::npos) brace_count++;
                if (line.find('}') != std::string::npos) brace_count--;
                if (brace_count > 0) func.body_lines.push_back(line);
            }
            m_functions[func.name] = func;
        } 
        else if (keyword == "instrument") {
            std::string instrument_name;
            ss >> instrument_name;
            
            Instrument template_inst;
            template_inst.name = instrument_name;
            
            std::string sub_line;
            int brace_count = 1;
            if (line.find('{') == std::string::npos) {
                while(std::getline(input_stream, sub_line) && sub_line.find('{') == std::string::npos);
            }

            while (std::getline(input_stream, sub_line) && brace_count > 0) {
                if (sub_line.find('{') != std::string::npos) brace_count++;
                if (sub_line.find('}') != std::string::npos) brace_count--;
                if (brace_count == 0) break;

                std::stringstream sub_ss(sub_line);
                std::string sub_kw;
                if (!(sub_ss >> sub_kw)) continue;

                if (sub_kw == "waveform") {
                    std::string w; sub_ss >> w;
                    if (w == "sine") template_inst.synth.waveform = Waveform::SINE;
                    else if (w == "square") template_inst.synth.waveform = Waveform::SQUARE;
                    else if (w == "triangle") template_inst.synth.waveform = Waveform::TRIANGLE;
                    else if (w == "sawtooth") template_inst.synth.waveform = Waveform::SAWTOOTH;
                } else if (sub_kw == "envelope") {
                    float a, d, s, r; sub_ss >> a >> d >> s >> r;
                    template_inst.synth.envelope = AdsrEnvelope(a, d, s, r);
                } else if (sub_kw == "sample") {
                    template_inst.type = InstrumentType::SAMPLER;
                    std::string path; std::getline(sub_ss, path);
                    const char q = '"';
                    size_t first = path.find_first_not_of(" \t\r\n");
                    if (first != std::string::npos && path[first] == q) first++;
                    size_t last = path.find_last_not_of(" \t\r\n");
                    if (last != std::string::npos && path[last] == q) last--;
                    if (first <= last) template_inst.sampler = new Sampler(path.substr(first, last - first + 1));
                } else if (sub_kw == "soundfont") {
                    template_inst.type = InstrumentType::SOUNDFONT;
                    std::string path; std::getline(sub_ss, path);
                    const char q = '"';
                    size_t first = path.find_first_not_of(" \t\r\n");
                    if (first != std::string::npos && path[first] == q) first++;
                    size_t last = path.find_last_not_of(" \t\r\n");
                    if (last != std::string::npos && path[last] == q) last--;
                    if (first <= last) template_inst.soundfont_path = path.substr(first, last - first + 1);
                } else if (sub_kw == "bank") {
                    sub_ss >> template_inst.bank_index;
                } else if (sub_kw == "preset") {
                    sub_ss >> template_inst.preset_index;
                } else if (sub_kw == "portamento") {
                    sub_ss >> template_inst.portamento_time;
                }
            }
            m_templates[instrument_name] = template_inst;
        }
    }
}

void ScriptParser::process_script_stream(std::istream& input_stream, const std::map<std::string, std::string>& current_param_map, std::shared_ptr<CompositeElement> current_parent) {
    std::string line;
    bool in_function_definition = false;
    bool in_instrument_definition = false;
    int def_brace_count = 0;

    while (std::getline(input_stream, line)) {
        if (!in_function_definition && !in_instrument_definition) {
            std::stringstream ss_check(line);
            std::string kw;
            if (ss_check >> kw) {
                if (kw == "function") { in_function_definition = true; def_brace_count = 1; continue; }
                if (kw == "instrument") { in_instrument_definition = true; def_brace_count = 1; continue; }
            }
        } else {
            if (line.find('{') != std::string::npos) def_brace_count++;
            if (line.find('}') != std::string::npos) def_brace_count--;
            if (def_brace_count == 0) { in_function_definition = false; in_instrument_definition = false; }
            continue;
        }

        line = substitute_params(line, current_param_map);
        std::stringstream ss(line);
        std::string keyword;
        if (!(ss >> keyword)) continue;

        if (keyword == "parallel") {
            auto parallel_elem = std::make_shared<CompositeElement>(CompositeType::PARALLEL);
            current_parent->children.push_back(parallel_elem);
            process_script_stream(input_stream, current_param_map, parallel_elem);
        } else if (keyword == "sequential") {
            auto sequential_elem = std::make_shared<CompositeElement>(CompositeType::SEQUENTIAL);
            current_parent->children.push_back(sequential_elem);
            process_script_stream(input_stream, current_param_map, sequential_elem);
        } else if (keyword == "repeat") {
            int count; ss >> count;
            std::vector<std::string> body;
            std::string sub_line;
            int brace_count = 0;
            if (line.find('{') != std::string::npos) brace_count = 1;
            else {
                while(std::getline(input_stream, sub_line) && sub_line.find('{') == std::string::npos);
                brace_count = 1;
            }
            while (brace_count > 0 && std::getline(input_stream, sub_line)) {
                if (sub_line.find('{') != std::string::npos) brace_count++;
                if (sub_line.find('}') != std::string::npos) brace_count--;
                if (brace_count > 0) body.push_back(sub_line);
            }
            for (int i = 0; i < count; ++i) {
                std::stringstream body_stream;
                for (const auto& bl : body) body_stream << bl << "\n";
                process_script_stream(body_stream, current_param_map, current_parent);
            }
        } else if (keyword == "call") {
            std::string rest; std::getline(ss, rest);
            size_t open_p = rest.find('(');
            size_t close_p = rest.find(')');
            std::string f_name = rest.substr(0, open_p);
            f_name.erase(std::remove_if(f_name.begin(), f_name.end(), ::isspace), f_name.end());

            if (m_functions.count(f_name)) {
                const auto& func = m_functions[f_name];
                std::string args_s = rest.substr(open_p + 1, close_p - open_p - 1);
                std::stringstream ass(args_s);
                std::string arg;
                std::map<std::string, std::string> next_map;
                size_t arg_i = 0;
                while (std::getline(ass, arg, ',') && arg_i < func.parameters.size()) {
                    arg.erase(std::remove_if(arg.begin(), arg.end(), ::isspace), arg.end());
                    next_map[func.parameters[arg_i++]] = arg;
                }
                std::stringstream body;
                for (const auto& l : func.body_lines) body << l << "\n";
                process_script_stream(body, next_map, current_parent);
            }
        } else if (keyword == "tempo") {
            float bpm; ss >> bpm;
            if (bpm > 0) m_default_duration = 60000 / bpm;
        } else if (keyword == "velocity") {
            ss >> m_default_velocity;
        } else if (m_templates.count(keyword)) {
            Instrument inst = m_templates[keyword];
            int inst_brace_count = 0;
            
            auto process_line = [&](std::stringstream& lss) {
                std::string lkw;
                while (lss >> lkw) {
                    if (lkw == "{") inst_brace_count++;
                    else if (lkw == "}") inst_brace_count--;
                    else if (lkw == "tempo") { float bpm; lss >> bpm; if (bpm > 0) m_default_duration = 60000 / bpm; }
                    else if (lkw == "velocity") { lss >> m_default_velocity; }
                    else if (lkw == "portamento") { lss >> inst.portamento_time; }
                    else if (lkw == "note") {
                        std::string n; int d, v; 
                        if (lss >> n >> d >> v) inst.sequence.add_note(Note(NoteParser::parse(n), d, v));
                        else inst.sequence.add_note(Note(NoteParser::parse(n), m_default_duration, m_default_velocity));
                    } else if (lkw == "notes") {
                        std::string note_list; std::getline(lss, note_list);
                        parse_compact_notes(note_list, inst.sequence);
                    }
                }
            };

            process_line(ss);

            std::string sub_line;
            while (inst_brace_count > 0 && std::getline(input_stream, sub_line)) {
                std::stringstream sub_ss(sub_line);
                process_line(sub_ss);
            }
            current_parent->children.push_back(std::make_shared<InstrumentElement>(inst));
        } else if (keyword == "}") {
            return;
        }
    }
}

std::string ScriptParser::substitute_params(const std::string& line, const std::map<std::string, std::string>& param_map) {
    std::string res = line;
    for (const auto& [p, v] : param_map) {
        std::string target = "$" + p;
        size_t pos = 0;
        while ((pos = res.find(target, pos)) != std::string::npos) {
            res.replace(pos, target.length(), v);
            pos += v.length();
        }
    }
    return res;
}

void ScriptParser::parse_compact_notes(const std::string& list, Sequence& seq) {
    std::string clean = list;
    int paren_level = 0;
    for (size_t i = 0; i < clean.length(); ++i) {
        if (clean[i] == '(') paren_level++;
        else if (clean[i] == ')') paren_level--;
        else if (clean[i] == ',' && paren_level == 0) clean[i] = ' ';
    }
    const std::string trim_chars = {' ', '\t', '\r', '\n', '"'};
    
    std::stringstream ss(clean);
    std::string token;
    while (ss >> token) {
        if (token.empty()) continue;
        if (token.find('(') != std::string::npos && token.find(')') == std::string::npos) {
            std::string next;
            while (ss >> next) {
                token += " " + next;
                if (next.find(')') != std::string::npos) break;
            }
        }
        size_t open_p = token.find('(');
        std::string note_name = token.substr(0, open_p);
        int dur = m_default_duration;
        int vel = m_default_velocity;
        if (open_p != std::string::npos) {
            size_t close_p = token.find(')');
            std::string params = token.substr(open_p + 1, close_p - open_p - 1);
            std::stringstream pss(params);
            int p1, p2;
            if (pss >> p1) {
                dur = p1;
                if (pss >> p2) vel = p2;
            }
        }
        int pitch = NoteParser::parse(note_name);
        if (pitch > 0 || note_name == "0") {
            seq.add_note(Note(pitch, dur, vel));
        }
    }
}
