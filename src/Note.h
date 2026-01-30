#ifndef NOTE_H
#define NOTE_H

class Note {
public:
    int pitch;
    int duration;
    int velocity;
    float pan; // -1.0 (left) to 1.0 (right)

    Note() = default;
    Note(int pitch, int duration, int velocity, float pan = 0.0f);
};

#endif // NOTE_H
