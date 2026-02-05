#ifndef NOTE_H
#define NOTE_H

class Note {
public:
    int pitch;
    int duration;
    int velocity;
    float pan; // -1.0 (left) to 1.0 (right)
    bool is_rest = false;
    bool advance_time = true;

    Note() = default;
    Note(int pitch, int duration, int velocity, float pan = 0.0f, bool advance_time = true);
};

#endif // NOTE_H