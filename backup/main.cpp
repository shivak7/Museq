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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <script_file> [--format <wav|mp3|ogg>]" << std::endl;
        return 1;
    }

    std::string script_file_path = argv[1];
    std::string format = "wav";

    std::vector<std::string> args(argv + 1, argv + argc);
    auto format_it = std::find(args.begin(), args.end(), "--format");
    if (format_it != args.end() && ++format_it != args.end()) {
        format = *format_it;
    } else {
        format_it = std::find(args.begin(), args.end(), "-f");
        if (format_it != args.end() && ++format_it != args.end()) {
            format = *format_it;
        }
    }


    Song song = ScriptParser::parse(script_file_path);
    std::cout << "Parsed script from " << script_file_path << std::endl;

    // Save the song
    JsonSerializer::save(song, "song.json");
    std::cout << "Saved song to song.json" << std::endl;

    AudioRenderer renderer;

    // Write the song to a file
    if (format == "wav") {
        WavWriter writer;
        writer.write(renderer, song, "song.wav");
        std::cout << "Rendered song to song.wav" << std::endl;
    } else if (format == "mp3") {
        Mp3Writer writer;
        writer.write(renderer, song, "song.mp3");
        std::cout << "Rendered song to song.mp3" << std::endl;
    } else if (format == "ogg") {
        OggWriter writer;
        writer.write(renderer, song, "song.ogg");
        std::cout << "Rendered song to song.ogg" << std::endl;
    } else {
        std::cerr << "Unsupported format: " << format << std::endl;
        return 1;
    }

    return 0;
}
