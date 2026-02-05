#include "Note.h"

Note::Note(int pitch, int duration, int velocity, float pan, bool advance_time) {
    this->pitch = pitch;
    this->duration = duration;
    this->velocity = velocity;
    this->pan = pan;
    this->advance_time = advance_time;
    this->is_rest = (pitch == -1);
}