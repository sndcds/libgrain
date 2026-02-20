//
//  JSONWalker.hpp
//
//  Created by Roald Christesen on from 15.02.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  Last viewed: 2026-02-15
//

#ifndef GrainJSONWalker_hpp
#define GrainJSONWalker_hpp

#include <fstream>
#include <iostream>
#include <string>
#include "Extern/nlohmann/json.hpp"


namespace Grain {

using json = nlohmann::json;

class JSONWalker {
public:
    virtual ~JSONWalker() = default;

    bool loadFromFile(const char* filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        try {
            json_root_ = json::parse(file);
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << "\n";
            return false;
        }
        return true;
    }


    void traverse() {
        if (!json_root_.is_null()) _traverseJSON(json_root_);
    }

protected:

    void _traverseJSON(const json& j) {
        walk(j, "");
    }

    void walk(const json& j, const std::string& path) {
        if (j.is_object()) {
            onObjectStart(path);

            for (auto it = j.begin(); it != j.end(); ++it) {
                std::string childPath = path.empty()
                    ? it.key()
                    : path + "." + it.key();

                walk(it.value(), childPath);
            }

            onObjectEnd(path);
        }
        else if (j.is_array()) {
            onArrayStart(path);

            for (size_t i = 0; i < j.size(); ++i) {
                std::string childPath =
                    path + "[" + std::to_string(i) + "]";

                walk(j[i], childPath);
            }

            onArrayEnd(path);
        }
        else {
            addProperty(path, j);
        }
    }

    // ---- Hooks you can override ----

    virtual void onObjectStart(const std::string& path) {}
    virtual void onObjectEnd(const std::string& path) {}

    virtual void onArrayStart(const std::string& path) {}
    virtual void onArrayEnd(const std::string& path) {}

    virtual void addProperty(const std::string& path, const json& value);


protected:
    json json_root_;
};

} // End of namespace Grain

#endif // GrainJSONWalker