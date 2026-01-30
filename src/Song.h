#ifndef SONG_H
#define SONG_H

#include <memory>
#include "SongElement.h"

class Song {
public:
    std::shared_ptr<CompositeElement> root;

    Song() {
        root = std::make_shared<CompositeElement>(CompositeType::SEQUENTIAL);
    }
};

#endif // SONG_H