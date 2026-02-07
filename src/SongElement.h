#ifndef SONG_ELEMENT_H
#define SONG_ELEMENT_H

#include <vector>
#include <memory>
#include <string>
#include "Instrument.h"

enum class CompositeType {
    SEQUENTIAL,
    PARALLEL,
    AUTO_LOOP
};

class SongElement {
public:
    int start_offset_ms = 0;
    virtual ~SongElement() = default;
    virtual double get_duration_ms() const = 0;
};

class InstrumentElement : public SongElement {
public:
    Instrument instrument;
    InstrumentElement(const Instrument& inst) : instrument(inst) {}
    
    double get_duration_ms() const override {
        double total = 0;
        for (const auto& note : instrument.sequence.notes) {
            total += note.duration;
        }
        return total;
    }
};

class CompositeElement : public SongElement {
public:
    CompositeType type;
    std::vector<std::shared_ptr<SongElement>> children;
    std::vector<Effect> effects;

    CompositeElement(CompositeType t) : type(t) {}

    double get_duration_ms() const override {
        double total = 0;
        if (type == CompositeType::SEQUENTIAL) {
            for (const auto& child : children) {
                total += child->start_offset_ms + child->get_duration_ms();
            }
        } else if (type == CompositeType::PARALLEL) {
            for (const auto& child : children) {
                double d = child->start_offset_ms + child->get_duration_ms();
                if (d > total) total = d;
            }
        } else if (type == CompositeType::AUTO_LOOP) {
            // Auto loop duration is defined by the leader (child 0)
            if (!children.empty()) {
                total = children[0]->start_offset_ms + children[0]->get_duration_ms();
            }
        }
        return total;
    }
};

#endif // SONG_ELEMENT_H