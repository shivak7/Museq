#include <iostream>
#include "Song.h"
#include "JsonSerializer.h"
#include "WavWriter.h"
#include "Mp3Writer.h"
#include "OggWriter.h"
#include "ScriptParser.h"
#include "AudioRenderer.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include "AudioPlayer.h"

void print_usage(const char* prog_name) {
    std::cerr << "Usage: " << prog_name << " <script_file> [options]" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -o, --out <file>      Specify output filename (extension ignored)" << std::endl;
    std::cerr << "  -f, --format <fmt>    Specify output format: wav, mp3, ogg (default: wav)" << std::endl;
    std::cerr << "  -q, --quality <rate>  Specify sample rate in Hz (default: 44100)" << std::endl;
    std::cerr << "  -p, --playback        Play directly to default speaker (ignores -o and -f)" << std::endl;
    std::cerr << "  -d, --dump-json       Dump the song structure to a JSON file" << std::endl;
    std::cerr << "  -Q, --query <sf2>     List instruments in a SoundFont file" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string script_file_path;
    std::string output_base_name = "song";
    std::string format = "wav";
    int sample_rate = 44100;
    bool playback_mode = false;
    bool dump_json = false;
    bool query_mode = false;
    std::string query_path;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" || arg == "--out") {
            if (i + 1 < argc) {
                output_base_name = argv[++i];
                size_t last_dot = output_base_name.find_last_of(".");
                if (last_dot != std::string::npos && last_dot > output_base_name.find_last_of("/\\")) {
                    output_base_name = output_base_name.substr(0, last_dot);
                }
            } else {
                std::cerr << "Error: Missing argument for output file." << std::endl;
                return 1;
            }
        } else if (arg == "-f" || arg == "--format") {
            if (i + 1 < argc) {
                format = argv[++i];
            } else {
                std::cerr << "Error: Missing argument for format." << std::endl;
                return 1;
            }
        } else if (arg == "-q" || arg == "--quality") {
            if (i + 1 < argc) {
                try {
                    sample_rate = std::stoi(argv[++i]);
                    if (sample_rate <= 0) throw std::invalid_argument("Invalid rate");
                } catch (...) {
                    std::cerr << "Error: Invalid sample rate." << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: Missing argument for sample rate." << std::endl;
                return 1;
            }
        } else if (arg == "-p" || arg == "--playback") {
            playback_mode = true;
        } else if (arg == "-d" || arg == "--dump-json") {
            dump_json = true;
        } else if (arg == "-Q" || arg == "--query") {
            query_mode = true;
            if (i + 1 < argc) {
                query_path = argv[++i];
            } else {
                std::cerr << "Error: Missing argument for query." << std::endl;
                return 1;
            }
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        } else {
            if (script_file_path.empty()) {
                script_file_path = arg;
            } else {
                std::cerr << "Error: Multiple input files specified." << std::endl;
                return 1;
            }
        }
    }

    if (query_mode) {
        AudioRenderer::print_soundfont_presets(query_path);
        return 0;
    }

    if (script_file_path.empty()) {
        std::cerr << "Error: No script file specified." << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    Song song = ScriptParser::parse(script_file_path);
    std::cout << "Parsed script from " << script_file_path << std::endl;

    if (dump_json) {
        std::string json_path = output_base_name + ".json";
        JsonSerializer::save(song, json_path);
        std::cout << "Saved song to " << json_path << std::endl;
    }

    AudioRenderer renderer;

    if (playback_mode) {
        std::cout << "Rendering and playing..." << std::endl;
        AudioPlayer player;
        if (player.init()) {
            player.play(song);
            std::cout << "Playing... Press Enter to stop." << std::endl;
            std::cin.get(); 
            player.stop();
        }
    } else {
        std::string output_file_path = output_base_name + "." + format;
        if (format == "wav") {
            WavWriter writer;
            writer.write(renderer, song, output_file_path, sample_rate);
            std::cout << "Rendered song to " << output_file_path << " at " << sample_rate << "Hz" << std::endl;
        } else if (format == "mp3") {
            Mp3Writer writer;
            writer.write(renderer, song, output_file_path, sample_rate);
            std::cout << "Rendered song to " << output_file_path << " at " << sample_rate << "Hz" << std::endl;
        } else if (format == "ogg") {
            OggWriter writer;
            writer.write(renderer, song, output_file_path, sample_rate);
            std::cout << "Rendered song to " << output_file_path << " at " << sample_rate << "Hz" << std::endl;
        } else {
            std::cerr << "Unsupported format: " << format << std::endl;
            return 1;
        }
    }

    return 0;
}