#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <vector>
#include "Note.h"

class Sequence {
public:
    std::vector<Note> notes;

    void add_note(Note note);
};

#endif // SEQUENCE_H
