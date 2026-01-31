#include "Note.h"

Note::Note(int pitch, int duration, int velocity, float pan) {
    this->pitch = pitch;
    this->duration = duration;
    this->velocity = velocity;
    this->pan = pan;
    this->is_rest = (pitch == -1);
}