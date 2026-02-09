#include "ScriptParser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "NoteParser.h"
#include "SongElement.h"
#include "Chord.h"

int ScriptParser::s_global_bpm = 120;

void ScriptParser::set_global_bpm(int bpm) {
    if (bpm > 0) s_global_bpm = bpm;
}

void ScriptParser::report_error(const std::string& message) {
    m_song.errors.push_back({m_current_line, message});
    std::cerr << "[" << m_filename << "] Line " << m_current_line << ": " << message << std::endl;
}

int ScriptParser::calculate_denominator_duration(int denominator) {
    // Valid denominators: Any positive integer (e.g. 4, 8, 5, 7, etc.)
    if (denominator <= 0) {
        return m_default_duration; // Fallback to current default
    }
    
    // Formula: Duration(ms) = (4.0 / Denominator) * (60000.0 / BPM)
    double beat_duration = 60000.0 / std::max(1, m_current_bpm);
    return static_cast<int>((4.0 / denominator) * beat_duration);
}

static std::string preprocess_line(const std::string& raw_line) {
    std::string line = raw_line;
    size_t comment_pos = line.find("//");
    if (comment_pos != std::string::npos) {
        line = line.substr(0, comment_pos);
    }
    
    std::string res;
    res.reserve(line.length() * 1.5);
    for (char c : line) {
        if (c == '{' || c == '}') {
            res += ' ';
            res += c;
            res += ' ';
        } else {
            res += c;
        }
    }
    return res;
}

ScriptParser::ScriptParser() {
    m_current_bpm = s_global_bpm;
    // Set default duration based on global BPM
    m_default_duration = 60000 / s_global_bpm;

    // Add built-in Rest instrument (produces silence)
    Instrument rest("Rest", Waveform::SINE, AdsrEnvelope(0, 0, 0, 0));
    m_templates["Rest"] = rest;
}

Song ScriptParser::parse(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open script file " << file_path << std::endl;
        return Song();
    }

    ScriptParser parser;
    parser.m_current_line = 0;
    parser.collect_definitions(file, file_path);
    file.close();
    file.open(file_path);
    if (!file.is_open()) return Song();
    
    parser.m_current_line = 0;
    std::map<std::string, std::string> empty_params;
    parser.process_script_stream(file, empty_params, parser.m_song.root, 0);

    return parser.m_song;
}

Song ScriptParser::parse_string(const std::string& script_content) {
    ScriptParser parser;
    parser.m_current_line = 0;
    std::stringstream collect_stream(script_content);
    parser.collect_definitions(collect_stream, "string_buffer");
    
    parser.m_current_line = 0;
    std::stringstream process_stream(script_content);
    std::map<std::string, std::string> empty_params;
    parser.process_script_stream(process_stream, empty_params, parser.m_song.root, 0);

    return parser.m_song;
}

void ScriptParser::collect_definitions(std::istream& input_stream, const std::string& filename) {
    std::string line;
    int scope_brace_count = 0;
    m_filename = filename;

    while (std::getline(input_stream, line)) {
        m_current_line++;
        line = preprocess_line(line);
        if (line.find_first_not_of(" 	\r\n") == std::string::npos) continue;

        if (line.find('{') != std::string::npos && line.find("function") == std::string::npos && line.find("instrument") == std::string::npos && line.find("sequence") == std::string::npos) scope_brace_count++;
        if (line.find('}') != std::string::npos) scope_brace_count--;
        if (scope_brace_count < 0) scope_brace_count = 0;

        std::stringstream ss(line);
        std::string keyword;
        if (!(ss >> keyword)) continue;

        if (scope_brace_count == 0 && keyword == "import") {
             std::string path_str; 
             if (std::getline(ss, path_str)) {
                 const std::string trim = {' ', '\t', '\r', '\n', '"'};
                 size_t first = path_str.find_first_not_of(trim);
                 size_t last = path_str.find_last_not_of(trim);
                 if (first != std::string::npos) {
                     path_str = path_str.substr(first, last - first + 1);
                     
                     // Resolve path:
                     std::filesystem::path resolved_path = std::filesystem::path(path_str);
                     if (!std::filesystem::exists(resolved_path)) {
                         std::filesystem::path base_path = std::filesystem::path(m_filename).parent_path();
                         resolved_path = base_path / path_str;
                     }
                     
                     std::string absolute_path = std::filesystem::absolute(resolved_path).string();

                     if (m_imported_files.find(absolute_path) == m_imported_files.end()) {
                         std::ifstream imported_file(resolved_path);
                         if (imported_file.is_open()) {
                             int saved_line = m_current_line;
                             std::string saved_filename = m_filename;
                             m_current_line = 0;
                             collect_definitions(imported_file, resolved_path.string());
                             m_current_line = saved_line;
                             m_filename = saved_filename;
                             m_imported_files.insert(absolute_path);
                         } else {
                             report_error("Could not open imported file: " + path_str);
                         }
                     }
                 }
             }
        }
        else if (scope_brace_count == 0 && keyword == "function") {
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
            while (brace_count > 0 && std::getline(input_stream, line)) {
                m_current_line++;
                std::string processed_line = preprocess_line(line);
                if (processed_line.find('{') != std::string::npos) brace_count++;
                if (processed_line.find('}') != std::string::npos) brace_count--;
                if (brace_count > 0) func.body_lines.push_back(line);
            }
            m_functions[func.name] = func;
        } 
        else if (scope_brace_count == 0 && keyword == "sequence") {
            FunctionDefinition func;
            func.is_sequence = true;
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
            while (brace_count > 0 && std::getline(input_stream, line)) {
                m_current_line++;
                std::string processed_line = preprocess_line(line);
                if (processed_line.find('{') != std::string::npos) brace_count++;
                if (processed_line.find('}') != std::string::npos) brace_count--;
                if (brace_count > 0) func.body_lines.push_back(line);
            }
            m_functions[func.name] = func;
        }
        else if (scope_brace_count == 0 && keyword == "var") {
            std::string name, val;
            if (ss >> name >> val) m_globals[name] = val;
        }
        else if (scope_brace_count == 0 && keyword == "scale") {
            // Scale definitions in templates
            std::string root, type_str;
            ss >> root >> type_str;
            ScaleType type = ScaleType::MAJOR;
            if (type_str == "major") type = ScaleType::MAJOR;
            else if (type_str == "minor") type = ScaleType::MINOR;
            else if (type_str == "dorian") type = ScaleType::DORIAN;
            else if (type_str == "phrygian") type = ScaleType::PHRYGIAN;
            else if (type_str == "lydian") type = ScaleType::LYDIAN;
            else if (type_str == "mixolydian") type = ScaleType::MIXOLYDIAN;
            else if (type_str == "locrian") type = ScaleType::LOCRIAN;
            m_current_scale = Scale(type, root);
        }
        else if (scope_brace_count == 0 && keyword == "instrument") {
            std::string instrument_name;
            ss >> instrument_name;
            int inst_start_line = m_current_line;
            
            std::string final_name = instrument_name;
            if (m_templates.count(final_name) > 0) {
                int suffix = 1;
                while (m_templates.count(instrument_name + "_" + std::to_string(suffix)) > 0) {
                    suffix++;
                }
                final_name = instrument_name + "_" + std::to_string(suffix);
                report_error("Instrument conflict. Renaming '" + instrument_name + "' to '" + final_name + "'");
            }

            Instrument template_inst;
            template_inst.name = final_name;
            template_inst.synth.filter = Filter(); 
            template_inst.synth.lfo = LFO();       
            
            std::string sub_line;
            int brace_count = 0;
            if (line.find('{') != std::string::npos) {
                brace_count = 1;
                // Handle single-line block
                if (line.find('}') != std::string::npos) {
                    brace_count = 0;
                    // Process the content between braces
                    size_t start = line.find('{') + 1;
                    size_t end = line.find('}', start);
                    std::string content = line.substr(start, end - start);
                    std::stringstream css(content);
                    std::string sub_kw;
                    while (css >> sub_kw) {
                        // Very simplified single-line parsing for collect_definitions
                        if (sub_kw == "waveform") {
                            std::string w; if (css >> w) {
                                if (w == "sine") template_inst.synth.waveform = Waveform::SINE;
                                else if (w == "square") template_inst.synth.waveform = Waveform::SQUARE;
                                else if (w == "triangle") template_inst.synth.waveform = Waveform::TRIANGLE;
                                else if (w == "sawtooth") template_inst.synth.waveform = Waveform::SAWTOOTH;
                            }
                        } else if (sub_kw == "sample" || sub_kw == "soundfont") {
                            template_inst.type = (sub_kw == "sample") ? InstrumentType::SAMPLER : InstrumentType::SOUNDFONT;
                        }
                    }
                }
            } else {
                while(std::getline(input_stream, sub_line)) {
                    m_current_line++;
                    if (sub_line.find('{') != std::string::npos) {
                        brace_count = 1;
                        break;
                    }
                }
            }

            bool in_sequence = false;

            while (brace_count > 0 && std::getline(input_stream, sub_line)) {
                m_current_line++;
                sub_line = preprocess_line(sub_line);
                if (sub_line.find('{') != std::string::npos) brace_count++;
                if (sub_line.find('}') != std::string::npos) {
                    brace_count--;
                    if (in_sequence && brace_count < 2) in_sequence = false;
                }
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
                    
                    float freq = 1.0f;
                    if (sub_ss >> freq) template_inst.synth.frequency = freq;
                } else if (sub_kw == "envelope") {
                    float a, d, s, r; sub_ss >> a >> d >> s >> r;
                    template_inst.synth.envelope = AdsrEnvelope(a, d, s, r);
                } else if (sub_kw == "filter") {
                    std::string type_str; float cutoff, res;
                    sub_ss >> type_str >> cutoff >> res;
                    if (type_str == "lowpass") template_inst.synth.filter.type = FilterType::LOWPASS;
                    else if (type_str == "highpass") template_inst.synth.filter.type = FilterType::HIGHPASS;
                    else if (type_str == "bandpass") template_inst.synth.filter.type = FilterType::BANDPASS;
                    template_inst.synth.filter.cutoff = cutoff;
                    template_inst.synth.filter.resonance = res;
                } else if (sub_kw == "lfo") {
                    std::string target_str, type_str; float freq, amt;
                    sub_ss >> target_str >> type_str >> freq >> amt;
                    if (target_str == "pitch") template_inst.synth.lfo.target = LFOTarget::PITCH;
                    else if (target_str == "amplitude") template_inst.synth.lfo.target = LFOTarget::AMPLITUDE;
                    else if (target_str == "cutoff") template_inst.synth.lfo.target = LFOTarget::FILTER_CUTOFF;
                    
                    if (type_str == "sine") template_inst.synth.lfo.waveform = Waveform::SINE;
                    else if (type_str == "square") template_inst.synth.lfo.waveform = Waveform::SQUARE;
                    else if (type_str == "triangle") template_inst.synth.lfo.waveform = Waveform::TRIANGLE;
                    else if (type_str == "sawtooth") template_inst.synth.lfo.waveform = Waveform::SAWTOOTH;
                    
                    template_inst.synth.lfo.frequency = freq;
                    template_inst.synth.lfo.amount = amt;
                } else if (sub_kw == "sample") {
                    template_inst.type = InstrumentType::SAMPLER;
                    std::string path; std::getline(sub_ss, path);
                    const std::string trim = {' ', '\t', '\r', '\n', '"'};
                    size_t first = path.find_first_not_of(trim);
                    size_t last = path.find_last_not_of(trim);
                    if (first != std::string::npos) template_inst.sampler = new Sampler(path.substr(first, last - first + 1));
                } else if (sub_kw == "soundfont") {
                    template_inst.type = InstrumentType::SOUNDFONT;
                    std::string path; std::getline(sub_ss, path);
                    const std::string trim = {' ', '\t', '\r', '\n', '"'};
                    size_t first = path.find_first_not_of(trim);
                    size_t last = path.find_last_not_of(trim);
                    if (first != std::string::npos) template_inst.soundfont_path = path.substr(first, last - first + 1);
                } else if (sub_kw == "bank") {
                    sub_ss >> template_inst.bank_index;
                } else if (sub_kw == "preset") {
                    sub_ss >> template_inst.preset_index;
                } else if (sub_kw == "portamento") {
                    sub_ss >> template_inst.portamento_time;
                } else if (sub_kw == "pan") {
                    sub_ss >> template_inst.pan;
                } else if (sub_kw == "gain") {
                    sub_ss >> template_inst.gain;
                } else if (sub_kw == "effect") {
                    std::string type_str; sub_ss >> type_str;
                    Effect fx;
                    if (type_str == "delay") { fx.type = EffectType::DELAY; sub_ss >> fx.param1 >> fx.param2; }
                    else if (type_str == "distortion") { fx.type = EffectType::DISTORTION; sub_ss >> fx.param1; }
                    else if (type_str == "bitcrush") { fx.type = EffectType::BITCRUSH; sub_ss >> fx.param1; }
                    else if (type_str == "fadein") { fx.type = EffectType::FADE_IN; sub_ss >> fx.param1; }
                    else if (type_str == "fadeout") { fx.type = EffectType::FADE_OUT; sub_ss >> fx.param1; }
                    else if (type_str == "tremolo") { fx.type = EffectType::TREMOLO; sub_ss >> fx.param1 >> fx.param2; }
                    else if (type_str == "reverb") { fx.type = EffectType::REVERB; sub_ss >> fx.param1 >> fx.param2; }
                    template_inst.effects.push_back(fx);
                } else if (sub_kw == "sequence") {
                    in_sequence = true;
                } else if (sub_kw == "note" && in_sequence) {
                    std::string n, d_s, v_s;
                    if (sub_ss >> n) {
                        int dur = m_default_duration;
                        size_t underscore_pos = n.find('_');
                        if (underscore_pos != std::string::npos) {
                            std::string denom_str = n.substr(underscore_pos + 1);
                            n = n.substr(0, underscore_pos);
                            try {
                                dur = calculate_denominator_duration(std::stoi(denom_str));
                            } catch(...) {}
                        }

                        if (sub_ss >> d_s >> v_s) {
                            if (d_s.find('.') != std::string::npos || v_s.find('.') != std::string::npos) {
                                report_error("Floating point value in 'note' duration/velocity. Skipping.");
                            } else {
                                try {
                                    template_inst.sequence.add_note(Note(NoteParser::parse(n), std::stoi(d_s), std::stoi(v_s)));
                                } catch(...) {}
                            }
                        } else {
                            template_inst.sequence.add_note(Note(NoteParser::parse(n), dur, m_default_velocity));
                        }
                    }
                } else if (sub_kw == "notes" && in_sequence) {
                    std::string note_list; std::getline(sub_ss, note_list);
                    parse_compact_notes(note_list, template_inst.sequence);
                }
            }
            m_templates[final_name] = template_inst;
        }
    }
}

bool ScriptParser::skipping_definition(const std::string& line, bool& in_function, bool& in_instrument, int& brace_count, std::istream& stream, int depth) {
    if (depth > 0) return false; 

    if (in_function || in_instrument) {
        if (line.find('{') != std::string::npos) brace_count++;
        if (line.find('}') != std::string::npos) brace_count--;
        if (brace_count == 0) {
            in_function = false;
            in_instrument = false;
        }
        return true;
    }

    std::stringstream ss(line);
    std::string kw;
    if (ss >> kw) {
        if (kw == "import") return true;
        if (kw == "function" || kw == "instrument" || kw == "sequence") {
            if (kw == "function" || kw == "sequence") in_function = true;
            else in_instrument = true;
            
            if (line.find('{') != std::string::npos) brace_count = 1;
            else {
                std::string next_line;
                while (std::getline(stream, next_line)) {
                    next_line = preprocess_line(next_line);
                    if (next_line.find('{') != std::string::npos) {
                        brace_count = 1;
                        break;
                    }
                }
            }
            if (line.find('}') != std::string::npos) brace_count--;
            
            if (brace_count == 0) {
                in_function = false;
                in_instrument = false;
            }
            return true;
        }
    }
    return false;
}

void ScriptParser::process_script_stream(std::istream& input_stream, const std::map<std::string, std::string>& current_param_map, std::shared_ptr<CompositeElement> current_parent, int depth) {
    // Update default duration based on current BPM
    if (m_current_bpm > 0) m_default_duration = 60000 / m_current_bpm;
    
    // For top-level call, m_filename is already set by collect_definitions.
    // For recursive calls (functions), we don't change m_filename as the lines 
    // are sourced from the original definition file.

    std::string line;
    bool in_function_definition = false;
    bool in_instrument_definition = false;
    int def_brace_count = 0;

    // Instrument parsing state variables
    std::string instrument_name;
    InstrumentType instrument_type = InstrumentType::SYNTH;
    Waveform waveform = Waveform::SINE;
    AdsrEnvelope envelope;
    Filter filter;
    LFO lfo;
    std::string sample_path;
    std::string soundfont_path;
    int bank_index = 0;
    int preset_index = 0;
    Sequence temp_sequence;
    bool in_instrument_block = false;
    bool in_sequence_block = false;
    std::vector<Effect> temp_effects;

    // Stack for loop context
    std::vector<std::shared_ptr<CompositeElement>> parent_stack;

    const std::string trim_chars = {' ', '\t', '\r', '\n', '"'};

    while (std::getline(input_stream, line)) {
        m_current_line++;
        line = preprocess_line(line);
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        if (skipping_definition(line, in_function_definition, in_instrument_definition, def_brace_count, input_stream, depth)) continue;

        line = substitute_params(line, current_param_map);
        std::stringstream ss(line);
        std::string keyword;
        if (!(ss >> keyword)) continue;
        
        if (keyword == "{") continue;
        if (keyword == "}") {
            if (depth > 0) return;
            continue;
        }

        if (isdigit(keyword[0])) {
            float timestamp = 0.0f;
            try {
                timestamp = std::stof(keyword);
            } catch (...) {}
            
            int offset_ms = static_cast<int>(timestamp * m_default_duration);
            
            std::string remainder;
            std::getline(ss, remainder);
            std::stringstream remainder_ss(remainder);
            
            size_t initial_size = current_parent->children.size();
            process_script_stream(remainder_ss, current_param_map, current_parent, depth + 1);
            
            for (size_t i = initial_size; i < current_parent->children.size(); ++i) {
                current_parent->children[i]->start_offset_ms += offset_ms;
            }
            continue;
        }

        if (keyword == "octave") {
            ss >> m_default_octave;
        } else if (keyword == "var") {
            std::string name, val;
            if (ss >> name >> val) m_globals[name] = val;
        } else if (keyword == "offset") {
            int offset_ms; ss >> offset_ms;
            int offset_line = m_current_line;
            std::vector<std::string> body;
            std::string sub_line;
            int brace_count = 0;
            if (line.find('{') != std::string::npos) brace_count = 1;
            else {
                while(std::getline(input_stream, sub_line)) {
                    m_current_line++;
                    if (sub_line.find('{') != std::string::npos) {
                        brace_count = 1;
                        break;
                    }
                }
            }
            while (brace_count > 0 && std::getline(input_stream, sub_line)) {
                m_current_line++;
                sub_line = preprocess_line(sub_line);
                if (sub_line.find('{') != std::string::npos) brace_count++;
                if (sub_line.find('}') != std::string::npos) brace_count--;
                if (brace_count > 0) body.push_back(sub_line);
            }
            
            auto temp_container = std::make_shared<CompositeElement>(CompositeType::SEQUENTIAL);
            temp_container->source_line = offset_line;
            std::stringstream body_stream;
            for (const auto& bl : body) body_stream << bl << "\n";
            process_script_stream(body_stream, current_param_map, temp_container, depth + 1);
            
            for (auto child : temp_container->children) {
                child->start_offset_ms += offset_ms;
                current_parent->children.push_back(child);
            }
        } else if (keyword == "phase") {
            float p; ss >> p;
            int offset_ms = static_cast<int>(p * m_default_duration);
            int phase_line = m_current_line;
            std::vector<std::string> body;
            std::string sub_line;
            int brace_count = 0;
            if (line.find('{') != std::string::npos) brace_count = 1;
            else {
                while(std::getline(input_stream, sub_line)) {
                    m_current_line++;
                    if (sub_line.find('{') != std::string::npos) {
                        brace_count = 1;
                        break;
                    }
                }
            }
            while (brace_count > 0 && std::getline(input_stream, sub_line)) {
                m_current_line++;
                sub_line = preprocess_line(sub_line);
                if (sub_line.find('{') != std::string::npos) brace_count++;
                if (sub_line.find('}') != std::string::npos) brace_count--;
                if (brace_count > 0) body.push_back(sub_line);
            }
            
            auto temp_container = std::make_shared<CompositeElement>(CompositeType::SEQUENTIAL);
            temp_container->source_line = phase_line;
            std::stringstream body_stream;
            for (const auto& bl : body) body_stream << bl << "\n";
            process_script_stream(body_stream, current_param_map, temp_container, depth + 1);
            
            for (auto child : temp_container->children) {
                child->start_offset_ms += offset_ms;
                current_parent->children.push_back(child);
            }
        } else if (keyword == "call") {
            std::string call_rest; std::getline(ss, call_rest);
            size_t open_p = call_rest.find('(');
            size_t close_p = call_rest.find(')');
            std::string f_name = call_rest.substr(0, open_p);
            f_name.erase(std::remove_if(f_name.begin(), f_name.end(), ::isspace), f_name.end());
            int call_line = m_current_line;

            if (m_functions.count(f_name)) {
                const auto& func = m_functions[f_name];
                std::string args_s = call_rest.substr(open_p + 1, close_p - open_p - 1);
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
                
                std::shared_ptr<CompositeElement> target_parent = current_parent;
                if (func.is_sequence) {
                    auto seq_elem = std::make_shared<CompositeElement>(CompositeType::PARALLEL);
                    seq_elem->source_line = call_line;
                    current_parent->children.push_back(seq_elem);
                    target_parent = seq_elem;
                }
                
                int saved_line = m_current_line;
                m_current_line = call_line - 1;
                process_script_stream(body, next_map, target_parent, depth + 1);
                m_current_line = saved_line;
            }
        } else if (keyword == "parallel") {
            auto parallel_elem = std::make_shared<CompositeElement>(CompositeType::PARALLEL);
            parallel_elem->source_line = m_current_line;
            current_parent->children.push_back(parallel_elem);
            process_script_stream(input_stream, current_param_map, parallel_elem, depth + 1);
        } else if (keyword == "sequential") {
            auto sequential_elem = std::make_shared<CompositeElement>(CompositeType::SEQUENTIAL);
            sequential_elem->source_line = m_current_line;
            current_parent->children.push_back(sequential_elem);
            process_script_stream(input_stream, current_param_map, sequential_elem, depth + 1);
        } else if (keyword == "repeat") {
            int count; ss >> count;
            int repeat_line = m_current_line;
            std::vector<std::string> body;
            std::string sub_line;
            int brace_count = 0;
            if (line.find('{') != std::string::npos) brace_count = 1;
            else {
                while(std::getline(input_stream, sub_line)) {
                    m_current_line++;
                    if (sub_line.find('{') != std::string::npos) {
                        brace_count = 1;
                        break;
                    }
                }
            }
            while (brace_count > 0 && std::getline(input_stream, sub_line)) {
                m_current_line++;
                if (sub_line.find('{') != std::string::npos) brace_count++;
                if (sub_line.find('}') != std::string::npos) brace_count--;
                if (brace_count > 0) body.push_back(sub_line);
            }
            for (int i = 0; i < count; ++i) {
                std::stringstream body_stream;
                for (const auto& bl : body) body_stream << bl << "\n";
                process_script_stream(body_stream, current_param_map, current_parent, depth + 1);
            }
        } else if (keyword == "loop") {
            std::string sub; ss >> sub;
            if (sub == "start") {
                int loop_line = m_current_line;
                std::string rest_of_line;
                std::getline(ss, rest_of_line);
                
                auto auto_loop = std::make_shared<CompositeElement>(CompositeType::AUTO_LOOP);
                auto_loop->source_line = loop_line;
                
                // Add followers (functions)
                std::stringstream rss(rest_of_line);
                std::string func_name;
                while (std::getline(rss, func_name, ',')) {
                    func_name.erase(std::remove_if(func_name.begin(), func_name.end(), ::isspace), func_name.end());
                    if (func_name.empty()) continue;
                    
                    if (m_functions.count(func_name)) {
                        const auto& func = m_functions[func_name];
                        auto follower = std::make_shared<CompositeElement>(CompositeType::SEQUENTIAL);
                        std::stringstream body;
                        for (const auto& l : func.body_lines) body << l << "\n";
                        std::map<std::string, std::string> empty_next_map; // No params supported for loop args yet
                        process_script_stream(body, empty_next_map, follower, depth + 1);
                        auto_loop->children.push_back(follower);
                    }
                }
                
                // Create Leader
                auto leader = std::make_shared<CompositeElement>(CompositeType::SEQUENTIAL);
                leader->source_line = loop_line;
                auto_loop->children.insert(auto_loop->children.begin(), leader); // Leader is Child 0
                
                current_parent->children.push_back(auto_loop);
                
                // Push stack
                parent_stack.push_back(current_parent);
                current_parent = leader;
                depth++;
                
            } else if (sub == "stop") {
                if (!parent_stack.empty()) {
                    current_parent = parent_stack.back();
                    parent_stack.pop_back();
                    depth--;
                }
            }
        } else if (keyword == "tempo") {
            float bpm; ss >> bpm;
            if (bpm > 0) {
                m_current_bpm = static_cast<int>(bpm);
                m_default_duration = 60000 / m_current_bpm;
            }
        } else if (keyword == "velocity") {
            ss >> m_default_velocity;
        } else if (keyword == "effect") {
            std::string type_str; ss >> type_str;
            Effect fx;
            if (type_str == "delay") { fx.type = EffectType::DELAY; ss >> fx.param1 >> fx.param2; }
            else if (type_str == "distortion") { fx.type = EffectType::DISTORTION; ss >> fx.param1; }
            else if (type_str == "bitcrush") { fx.type = EffectType::BITCRUSH; ss >> fx.param1; }
            else if (type_str == "fadein") { fx.type = EffectType::FADE_IN; ss >> fx.param1; }
            else if (type_str == "fadeout") { fx.type = EffectType::FADE_OUT; ss >> fx.param1; }
            else if (type_str == "tremolo") { fx.type = EffectType::TREMOLO; ss >> fx.param1 >> fx.param2; }
            else if (type_str == "reverb") { fx.type = EffectType::REVERB; ss >> fx.param1 >> fx.param2; }
            
            if (fx.type != EffectType::NONE) {
                current_parent->effects.push_back(fx);
            }
        } else if (keyword == "scale") {
            std::string root, type_str;
            ss >> root >> type_str;
            ScaleType type = ScaleType::MAJOR;
            if (type_str == "major") type = ScaleType::MAJOR;
            else if (type_str == "minor") type = ScaleType::MINOR;
            else if (type_str == "dorian") type = ScaleType::DORIAN;
            else if (type_str == "phrygian") type = ScaleType::PHRYGIAN;
            else if (type_str == "lydian") type = ScaleType::LYDIAN;
            else if (type_str == "mixolydian") type = ScaleType::MIXOLYDIAN;
            else if (type_str == "locrian") type = ScaleType::LOCRIAN;
            m_current_scale = Scale(type, root);
        } else if (m_templates.count(keyword)) {
            Instrument inst = m_templates[keyword];
            int inst_brace_count = 0;
            int current_octave = m_default_octave;
            int inst_line = m_current_line;
            
            auto process_inst_line = [&](std::string l) {
                if (l.find('{') != std::string::npos) inst_brace_count++;
                if (l.find('}') != std::string::npos) inst_brace_count--;
                
                std::stringstream lss(l);
                std::string lkw;
                while (lss >> lkw) {
                    if (lkw == "{" || lkw == "}") continue;
                    if (lkw == "note") {
                        std::string n;
                        if (lss >> n) {
                            // Handle parentheses grouping (e.g., C(500, 100))
                            if (n.find('(') != std::string::npos && n.find(')') == std::string::npos) {
                                std::string next;
                                while (lss >> next) {
                                    n += " " + next;
                                    if (next.find(')') != std::string::npos) break;
                                }
                            }

                            int dur = m_default_duration;
                            int vel = m_default_velocity;
                            float pan = inst.pan;
                            bool params_parsed = false;

                            size_t open_p = n.find('(');
                            if (open_p != std::string::npos) {
                                size_t close_p = n.find(')');
                                if (close_p != std::string::npos) {
                                     std::string p_str = n.substr(open_p + 1, close_p - open_p - 1);
                                     n = n.substr(0, open_p);
                                     std::replace(p_str.begin(), p_str.end(), ',', ' ');
                                     std::stringstream pss(p_str);
                                     int p1; 
                                     if (pss >> p1) {
                                         dur = p1;
                                         int p2;
                                         if (pss >> p2) {
                                             vel = p2;
                                             float p3;
                                             if (pss >> p3) pan = p3;
                                         }
                                     }
                                     params_parsed = true;
                                }
                            }

                            size_t underscore_pos = n.find('_');
                            if (underscore_pos != std::string::npos) {
                                std::string denom_str = n.substr(underscore_pos + 1);
                                n = n.substr(0, underscore_pos);
                                try {
                                    int d = calculate_denominator_duration(std::stoi(denom_str));
                                    if (!params_parsed) dur = d;
                                } catch(...) {
                                    report_error("Invalid denominator suffix: " + denom_str);
                                }
                            }

                            if (!params_parsed) {
                                std::string d_s, v_s, p_s;
                                if (lss >> d_s && lss >> v_s) {
                                    if (d_s.find('.') != std::string::npos || v_s.find('.') != std::string::npos) {
                                        report_error("Floating point value in 'note' duration/velocity. Skipping.");
                                    } else {
                                        try {
                                            dur = std::stoi(d_s);
                                            vel = std::stoi(v_s);
                                            if (lss >> p_s) {
                                                if (p_s != "}" && p_s != "{" && (isdigit(p_s[0]) || p_s[0] == '-' || p_s[0] == '.')) {
                                                    pan = std::stof(p_s);
                                                }
                                            }
                                        } catch(...) {
                                            report_error("Failed to parse note params for " + n);
                                        }
                                    }
                                }
                            }
                            
                            Note new_note(NoteParser::parse(n, m_current_scale, current_octave), dur, vel, pan);
                            inst.sequence.add_note(new_note);
                        }
                    } else if (lkw == "notes") {
                        std::string note_list; std::getline(lss, note_list);
                        parse_compact_notes(note_list, inst.sequence, inst.pan, current_octave);
                    } else if (lkw == "pan") {
                        lss >> inst.pan;
                    } else if (lkw == "gain") {
                        lss >> inst.gain;
                    } else if (lkw == "octave") {
                        lss >> current_octave;
                    } else if (lkw == "effect") {
                        std::string type_str; lss >> type_str;
                        Effect fx;
                        if (type_str == "delay") { fx.type = EffectType::DELAY; lss >> fx.param1 >> fx.param2; }
                        else if (type_str == "distortion") { fx.type = EffectType::DISTORTION; lss >> fx.param1; }
                        else if (type_str == "bitcrush") { fx.type = EffectType::BITCRUSH; lss >> fx.param1; }
                        else if (type_str == "fadein") { fx.type = EffectType::FADE_IN; lss >> fx.param1; }
                        else if (type_str == "fadeout") { fx.type = EffectType::FADE_OUT; lss >> fx.param1; }
                        else if (type_str == "tremolo") { fx.type = EffectType::TREMOLO; lss >> fx.param1 >> fx.param2; }
                        else if (type_str == "reverb") { fx.type = EffectType::REVERB; lss >> fx.param1 >> fx.param2; }
                        inst.effects.push_back(fx);
                    }
                }
            };

            std::string remainder;
            std::getline(ss, remainder);
            process_inst_line(remainder);

            std::string sub_line;
            while (inst_brace_count > 0 && std::getline(input_stream, sub_line)) {
                m_current_line++;
                sub_line = preprocess_line(sub_line);
                process_inst_line(sub_line);
            }
            auto inst_elem = std::make_shared<InstrumentElement>(inst);
            inst_elem->source_line = inst_line;
            current_parent->children.push_back(inst_elem);
        } else if (m_functions.count(keyword)) {
            // Implicit function call
            const auto& func = m_functions[keyword];
            std::map<std::string, std::string> next_map;
            int call_line = m_current_line;
            
            // Try to parse arguments if provided in parentheses
            std::string remainder;
            std::getline(ss, remainder);
            size_t open_p = remainder.find('(');
            size_t close_p = remainder.find(')');
            if (open_p != std::string::npos && close_p != std::string::npos) {
                std::string args_s = remainder.substr(open_p + 1, close_p - open_p - 1);
                std::stringstream ass(args_s);
                std::string arg;
                size_t arg_i = 0;
                while (std::getline(ass, arg, ',') && arg_i < func.parameters.size()) {
                    arg.erase(std::remove_if(arg.begin(), arg.end(), ::isspace), arg.end());
                    next_map[func.parameters[arg_i++]] = arg;
                }
            }
            
            std::stringstream body;
            for (const auto& l : func.body_lines) body << l << "\n";
            
            std::shared_ptr<CompositeElement> target_parent = current_parent;
            if (func.is_sequence) {
                auto seq_elem = std::make_shared<CompositeElement>(CompositeType::PARALLEL);
                seq_elem->source_line = call_line;
                current_parent->children.push_back(seq_elem);
                target_parent = seq_elem;
            }
            
            // Temporary override current line to the call site for all elements in this function
            int saved_line = m_current_line;
            m_current_line = call_line - 1; // It will be incremented in the next process_script_stream
            process_script_stream(body, next_map, target_parent, depth + 1);
            m_current_line = saved_line;
        } else if (keyword == "}") {
            if (in_sequence_block) {
                in_sequence_block = false;
            } else if (in_instrument_block) {
                Instrument i;
                if (instrument_type == InstrumentType::SYNTH) i = Instrument(instrument_name, waveform, envelope);
                else if (instrument_type == InstrumentType::SAMPLER) i = Instrument(instrument_name, sample_path);
                else if (instrument_type == InstrumentType::SOUNDFONT) i = Instrument(instrument_name, soundfont_path, bank_index, preset_index);
                i.sequence = temp_sequence;
                i.synth.filter = filter;
                i.synth.lfo = lfo;
                i.effects = temp_effects; // Copy effects
                auto inst_elem = std::make_shared<InstrumentElement>(i);
                inst_elem->source_line = m_current_line; // Might be better to track start line
                current_parent->children.push_back(inst_elem);
                in_instrument_block = false;
            } else {
                if (depth > 0) return;
            }
        } else {
            report_error("Unknown keyword or instrument: " + keyword);
        }
    }
}

std::string ScriptParser::substitute_params(const std::string& line, const std::map<std::string, std::string>& param_map) {
    std::string res = line;
    // Local parameters (highest priority)
    for (const auto& [p, v] : param_map) {
        std::string target = "$" + p;
        size_t pos = 0;
        while ((pos = res.find(target, pos)) != std::string::npos) {
            res.replace(pos, target.length(), v);
            pos += v.length();
        }
    }
    // Global variables
    for (const auto& [p, v] : m_globals) {
        std::string target = "$" + p;
        size_t pos = 0;
        while ((pos = res.find(target, pos)) != std::string::npos) {
            res.replace(pos, target.length(), v);
            pos += v.length();
        }
    }
    return res;
}

void ScriptParser::parse_compact_notes(const std::string& list, Sequence& seq, float default_pan, int default_octave) {
    std::string clean = substitute_params(list, {});
    int paren_level = 0;
    for (size_t i = 0; i < clean.length(); ++i) {
        if (clean[i] == '(') paren_level++;
        else if (clean[i] == ')') paren_level--;
        else if (clean[i] == ',' && paren_level == 0) clean[i] = ' ';
    }
    
    std::stringstream ss(clean);
    std::string raw_token;
    while (ss >> raw_token) {
        if (raw_token.empty() || raw_token == "}" || raw_token == "{") continue;
        
        // Strip trailing braces if they are attached to the last note
        if (raw_token.back() == '}') {
            raw_token.pop_back();
            if (raw_token.empty()) continue;
        }

        // Handle parentheses grouping (e.g., C(500))
        if (raw_token.find('(') != std::string::npos && raw_token.find(')') == std::string::npos) {
            std::string next;
            while (ss >> next) {
                raw_token += " " + next;
                if (next.find(')') != std::string::npos) break;
            }
        }

        // Handle Repetition (Feature 3)
        std::string token_body = raw_token;
        int repeat_count = 1;
        
        size_t star_pos = raw_token.find('*');
        // Check if * is outside parentheses
        if (star_pos != std::string::npos) {
             size_t open_p = raw_token.find('(');
             size_t close_p = raw_token.find(')');
             if (open_p == std::string::npos || star_pos < open_p || star_pos > close_p) {
                 // Check if it's really a multiplier (digits after *)
                 std::string count_str = raw_token.substr(star_pos + 1);
                 if (!count_str.empty() && std::all_of(count_str.begin(), count_str.end(), ::isdigit)) {
                     try {
                         repeat_count = std::stoi(count_str);
                         token_body = raw_token.substr(0, star_pos);
                     } catch(...) {}
                 }
             }
        }

        // Handle Chords (Feature: True Polyphony)
        std::vector<std::string> chord_components;
        std::stringstream css(token_body);
        std::string component;
        while (std::getline(css, component, '+')) {
            chord_components.push_back(component);
        }

        for (int r = 0; r < repeat_count; ++r) {
            for (size_t i = 0; i < chord_components.size(); ++i) {
                const std::string& comp = chord_components[i];
                bool is_last = (i == chord_components.size() - 1);

                size_t open_p = comp.find('(');
                size_t underscore_pos = comp.find('_');
                std::string note_name = comp.substr(0, std::min(open_p, underscore_pos));
                int dur = m_default_duration;
                int vel = m_default_velocity;
                float p_val = default_pan; 

                if (underscore_pos != std::string::npos && (open_p == std::string::npos || underscore_pos < open_p)) {
                    std::string denom_str;
                    if (open_p != std::string::npos) {
                        denom_str = comp.substr(underscore_pos + 1, open_p - underscore_pos - 1);
                    } else {
                        denom_str = comp.substr(underscore_pos + 1);
                    }
                    try {
                        dur = calculate_denominator_duration(std::stoi(denom_str));
                    } catch(...) {
                        report_error("Invalid denominator in 'notes' sequence: " + denom_str);
                    }
                }

                if (open_p != std::string::npos) {
                    size_t close_p = comp.find(')');
                    std::string params_str = comp.substr(open_p + 1, close_p - open_p - 1);
                    
                    // Validation: Check for invalid floating point in dur/vel
                    size_t first_comma = params_str.find(',');
                    size_t second_comma = (first_comma != std::string::npos) ? params_str.find(',', first_comma + 1) : std::string::npos;
                    
                    std::string dur_vel_part = (second_comma != std::string::npos) ? params_str.substr(0, second_comma) : params_str;
                    if (dur_vel_part.find('.') != std::string::npos) {
                        report_error("Floating point value detected in duration/velocity of 'notes' sequence. Skipping note: " + comp);
                        continue;
                    }

                    std::string params_clean = params_str;
                    std::replace(params_clean.begin(), params_clean.end(), ',', ' ');
                    
                    std::stringstream pss(params_clean);
                    int p1 = 0, p2 = 0; float p3 = 0.0f;
                    if (pss >> p1) {
                        dur = p1;
                        if (pss >> p2) {
                            vel = p2;
                            if (pss >> p3) p_val = p3;
                        }
                    } else if (!params_clean.empty()) {
                        report_error("Invalid parameters in 'notes' sequence. Skipping note: " + comp);
                        continue;
                    }
                }
                
                // NEW: Try to parse as a chord name (e.g. Cmaj, Dmin7)
                if (Chord::is_chord(note_name)) {
                    Chord chord(note_name);
                    std::vector<int> chord_pitches = chord.get_notes(default_octave);
                    if (!chord_pitches.empty()) {
                        for (size_t k = 0; k < chord_pitches.size(); ++k) {
                            bool is_last_chord_note = (k == chord_pitches.size() - 1);
                            seq.add_note(Note(chord_pitches[k], dur, vel, p_val, is_last_chord_note));
                        }
                        continue; // Processed as chord name
                    }
                }

                int pitch = NoteParser::parse(note_name, m_current_scale, default_octave);
                if (pitch > 0 || note_name == "0" || pitch == -1) {
                    seq.add_note(Note(pitch, dur, vel, p_val, is_last));
                } else {
                    report_error("Invalid note name '" + note_name + "' in sequence. Skipping.");
                }
            }
        }
    }
}