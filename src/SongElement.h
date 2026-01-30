#ifndef SONG_ELEMENT_H
#define SONG_ELEMENT_H

#include <vector>
#include <memory>
#include <string>
#include "Instrument.h"

enum class CompositeType {
    SEQUENTIAL,
    PARALLEL
};

class SongElement {
public:
    virtual ~SongElement() = default;
};

class InstrumentElement : public SongElement {
public:
    Instrument instrument;
    InstrumentElement(const Instrument& inst) : instrument(inst) {}
};

class CompositeElement : public SongElement {
public:
    CompositeType type;
    std::vector<std::shared_ptr<SongElement>> children;

    CompositeElement(CompositeType t) : type(t) {}
};

#endif // SONG_ELEMENT_H
