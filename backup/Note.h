#ifndef NOTE_H
#define NOTE_H

class Note {
public:
    int pitch;
    int duration;
    int velocity;

    Note() = default;
    Note(int pitch, int duration, int velocity);
};

#endif // NOTE_H
