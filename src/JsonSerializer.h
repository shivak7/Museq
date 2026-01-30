#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include "Song.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class JsonSerializer {
public:
    static void save(const Song& song, const std::string& file_path);
    static Song load(const std::string& file_path);
};

#endif // JSON_SERIALIZER_H
