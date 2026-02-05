#ifndef NOTE_PARSER_H
#define NOTE_PARSER_H

#include <string>

class NoteParser {
public:
    static int parse(const std::string& note_name, int default_octave = 4);
};

#endif // NOTE_PARSER_H
