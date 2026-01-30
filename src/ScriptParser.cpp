#include "ScriptParser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "NoteParser.h"
#include "SongElement.h"

static std::string remove_comments(const std::string& line) {
    size_t comment_pos = line.find("//");
    if (comment_pos != std::string::npos) {
        return line.substr(0, comment_pos);
    }
    return line;
}

ScriptParser::ScriptParser() {}

Song ScriptParser::parse(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open script file " << file_path << std::endl;
        return Song();
    }

    ScriptParser parser;
    parser.collect_definitions(file);
    file.close();
    file.open(file_path);
    if (!file.is_open()) return Song();
    
    std::map<std::string, std::string> empty_params;
    parser.process_script_stream(file, empty_params, parser.m_song.root, 0);

    return parser.m_song;
}

void ScriptParser::collect_definitions(std::istream& input_stream) {
    std::string line;
    int scope_brace_count = 0;

    while (std::getline(input_stream, line)) {
        line = remove_comments(line);
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        if (line.find('{') != std::string::npos && line.find("function") == std::string::npos && line.find("instrument") == std::string::npos) scope_brace_count++;
        if (line.find('}') != std::string::npos) scope_brace_count--;
        if (scope_brace_count < 0) scope_brace_count = 0;

        std::stringstream ss(line);
        std::string keyword;
        if (!(ss >> keyword)) continue;

        if (scope_brace_count == 0 && keyword == "function") {
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
        else if (scope_brace_count == 0 && keyword == "instrument") {
            std::string instrument_name;
            ss >> instrument_name;
            
            Instrument template_inst;
            template_inst.name = instrument_name;
            template_inst.synth.filter = Filter(); // Ensure default
            template_inst.synth.lfo = LFO();       // Ensure default
            
            std::string sub_line;
            int brace_count = 0;
            if (line.find('{') != std::string::npos) brace_count = 1;
            else {
                while(std::getline(input_stream, sub_line) && sub_line.find('{') == std::string::npos);
                brace_count = 1;
            }

            bool in_sequence = false;

            while (brace_count > 0 && std::getline(input_stream, sub_line)) {
                sub_line = remove_comments(sub_line);
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
                    const std::string trim = " \t\r\n\"";
                    size_t first = path.find_first_not_of(trim);
                    size_t last = path.find_last_not_of(trim);
                    if (first != std::string::npos) template_inst.sampler = new Sampler(path.substr(first, last - first + 1));
                } else if (sub_kw == "soundfont") {
                    template_inst.type = InstrumentType::SOUNDFONT;
                    std::string path; std::getline(sub_ss, path);
                    const std::string trim = " \t\r\n\"";
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
                } else if (sub_kw == "sequence") {
                    in_sequence = true;
                } else if (sub_kw == "note" && in_sequence) {
                    std::string n; int d, v; sub_ss >> n >> d >> v;
                    template_inst.sequence.add_note(Note(NoteParser::parse(n), d, v));
                } else if (sub_kw == "notes" && in_sequence) {
                    std::string note_list; std::getline(sub_ss, note_list);
                    parse_compact_notes(note_list, template_inst.sequence);
                }
            }
            m_templates[instrument_name] = template_inst;
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
        if (kw == "function" || kw == "instrument") {
            if (kw == "function") in_function = true;
            else in_instrument = true;
            
            if (line.find('{') != std::string::npos) brace_count = 1;
            else {
                std::string next_line;
                while (std::getline(stream, next_line)) {
                    next_line = remove_comments(next_line);
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

    const std::string trim_chars = " \t\r\n\"";

    while (std::getline(input_stream, line)) {
        line = remove_comments(line);
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        if (skipping_definition(line, in_function_definition, in_instrument_definition, def_brace_count, input_stream, depth)) continue;

        line = substitute_params(line, current_param_map);
        std::stringstream ss(line);
        std::string keyword;
        if (!(ss >> keyword)) continue;

        if (keyword == "instrument") {
            ss >> instrument_name;
            instrument_type = InstrumentType::SYNTH;
            waveform = Waveform::SINE;
            envelope = AdsrEnvelope();
            filter = Filter();
            lfo = LFO();
            sample_path = "";
            soundfont_path = "";
            bank_index = 0;
            preset_index = 0;
            temp_sequence = Sequence();
            in_instrument_block = true;
        } else if (keyword == "waveform" && in_instrument_block) {
            std::string w; ss >> w;
            if (w == "sine") waveform = Waveform::SINE;
            else if (w == "square") waveform = Waveform::SQUARE;
            else if (w == "triangle") waveform = Waveform::TRIANGLE;
            else if (w == "sawtooth") waveform = Waveform::SAWTOOTH;
        } else if (keyword == "envelope" && in_instrument_block) {
            float a, d, s, r; ss >> a >> d >> s >> r;
            envelope = AdsrEnvelope(a, d, s, r);
        } else if (keyword == "filter" && in_instrument_block) {
            std::string type_str; float cutoff, res;
            ss >> type_str >> cutoff >> res;
            if (type_str == "lowpass") filter.type = FilterType::LOWPASS;
            else if (type_str == "highpass") filter.type = FilterType::HIGHPASS;
            else if (type_str == "bandpass") filter.type = FilterType::BANDPASS;
            filter.cutoff = cutoff;
            filter.resonance = res;
        } else if (keyword == "lfo" && in_instrument_block) {
            std::string target_str, type_str; float freq, amt;
            ss >> target_str >> type_str >> freq >> amt;
            if (target_str == "pitch") lfo.target = LFOTarget::PITCH;
            else if (target_str == "amplitude") lfo.target = LFOTarget::AMPLITUDE;
            else if (target_str == "cutoff") lfo.target = LFOTarget::FILTER_CUTOFF;
            
            if (type_str == "sine") lfo.waveform = Waveform::SINE;
            else if (type_str == "square") lfo.waveform = Waveform::SQUARE;
            else if (type_str == "triangle") lfo.waveform = Waveform::TRIANGLE;
            else if (type_str == "sawtooth") lfo.waveform = Waveform::SAWTOOTH;
            
            lfo.frequency = freq;
            lfo.amount = amt;
        } else if (keyword == "sample" && in_instrument_block) {
            instrument_type = InstrumentType::SAMPLER;
            std::getline(ss, sample_path);
            size_t f = sample_path.find_first_not_of(trim_chars);
            size_t l = sample_path.find_last_not_of(trim_chars);
            if (f != std::string::npos) sample_path = sample_path.substr(f, (l - f + 1));
        } else if (keyword == "soundfont" && in_instrument_block) {
            instrument_type = InstrumentType::SOUNDFONT;
            std::getline(ss, soundfont_path);
            size_t f = soundfont_path.find_first_not_of(trim_chars);
            size_t l = soundfont_path.find_last_not_of(trim_chars);
            if (f != std::string::npos) soundfont_path = soundfont_path.substr(f, (l - f + 1));
        } else if (keyword == "bank" && in_instrument_block) {
            ss >> bank_index;
        } else if (keyword == "preset" && in_instrument_block) {
            ss >> preset_index;
        } else if (keyword == "sequence" && in_instrument_block) {
            in_sequence_block = true;
        } else if (keyword == "note" && in_sequence_block) {
            std::string n; int d, v; ss >> n >> d >> v;
            temp_sequence.add_note(Note(NoteParser::parse(n), d, v));
        } else if (keyword == "call") {
            std::string call_rest; std::getline(ss, call_rest);
            size_t open_p = call_rest.find('(');
            size_t close_p = call_rest.find(')');
            std::string f_name = call_rest.substr(0, open_p);
            f_name.erase(std::remove_if(f_name.begin(), f_name.end(), ::isspace), f_name.end());

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
                process_script_stream(body, next_map, current_parent, depth + 1);
            }
        } else if (keyword == "parallel") {
            auto parallel_elem = std::make_shared<CompositeElement>(CompositeType::PARALLEL);
            current_parent->children.push_back(parallel_elem);
            process_script_stream(input_stream, current_param_map, parallel_elem, depth + 1);
        } else if (keyword == "sequential") {
            auto sequential_elem = std::make_shared<CompositeElement>(CompositeType::SEQUENTIAL);
            current_parent->children.push_back(sequential_elem);
            process_script_stream(input_stream, current_param_map, sequential_elem, depth + 1);
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
                process_script_stream(body_stream, current_param_map, current_parent, depth + 1);
            }
        } else if (keyword == "tempo") {
            float bpm; ss >> bpm;
            if (bpm > 0) m_default_duration = 60000 / bpm;
        } else if (keyword == "velocity") {
            ss >> m_default_velocity;
        } else if (m_templates.count(keyword)) {
            Instrument inst = m_templates[keyword];
            int inst_brace_count = 0;
            
            auto process_inst_line = [&](std::string l) {
                if (l.find('{') != std::string::npos) inst_brace_count++;
                if (l.find('}') != std::string::npos) inst_brace_count--;
                
                std::stringstream lss(l);
                std::string lkw;
                while (lss >> lkw) {
                    if (lkw == "{" || lkw == "}") continue;
                    if (lkw == "tempo") { float bpm; lss >> bpm; if (bpm > 0) m_default_duration = 60000 / bpm; }
                    else if (lkw == "velocity") { lss >> m_default_velocity; }
                    else if (lkw == "portamento") { lss >> inst.portamento_time; }
                    else if (lkw == "pan") { lss >> inst.pan; }
                    else if (lkw == "note") {
                        std::string n; int d, v; float p;
                        if (lss >> n >> d >> v >> p) inst.sequence.add_note(Note(NoteParser::parse(n), d, v, p));
                        else if (lss >> n >> d >> v) inst.sequence.add_note(Note(NoteParser::parse(n), d, v, inst.pan));
                        else inst.sequence.add_note(Note(NoteParser::parse(n), m_default_duration, m_default_velocity, inst.pan));
                    } else if (lkw == "notes") {
                        std::string note_list; std::getline(lss, note_list);
                        parse_compact_notes(note_list, inst.sequence);
                    }
                }
            };

            std::string remainder;
            std::getline(ss, remainder);
            process_inst_line(remainder);

            std::string sub_line;
            while (inst_brace_count > 0 && std::getline(input_stream, sub_line)) {
                process_inst_line(sub_line);
            }
            current_parent->children.push_back(std::make_shared<InstrumentElement>(inst));
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
                current_parent->children.push_back(std::make_shared<InstrumentElement>(i));
                in_instrument_block = false;
            } else {
                return;
            }
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
    const std::string trim_chars = " \t\r\n\"";
    
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
        float p_val = 0.0f; 

        if (open_p != std::string::npos) {
            size_t close_p = token.find(')');
            std::string params = token.substr(open_p + 1, close_p - open_p - 1);
            std::replace(params.begin(), params.end(), ',', ' ');
            
            std::stringstream pss(params);
            int p1, p2; float p3;
            if (pss >> p1) {
                dur = p1;
                if (pss >> p2) {
                    vel = p2;
                    if (pss >> p3) p_val = p3;
                }
            }
        }
        int pitch = NoteParser::parse(note_name);
        if (pitch > 0 || note_name == "0") {
            seq.add_note(Note(pitch, dur, vel, p_val));
        }
    }
}