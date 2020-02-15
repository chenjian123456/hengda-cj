#pragma once

#include "border.h"
#include "json11.hpp"

#include <string>
#include <map>
#include <vector>

class BordersLoader {
public:
    static BordersLoader& Instance() {
        static BordersLoader loader;
        return loader;
    }

    void save(const json11::Json& borders,const std::string filename);

    std::map<std::string, std::vector<Border>> load();
    std::vector<Border> load(std::string filename);

private:
    BordersLoader() = default;

    std::string borders_file_ = "borders.json";
    json11::Json borders_;
};
