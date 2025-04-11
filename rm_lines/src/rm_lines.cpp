#include "rm_lines.h"
#include "v5/reader.h"
#include "v6/reader.h"
#include <cstring>
#include <uuid/uuid.h>
#include <cstddef>
#include <cstdio>
#include <format>
#include <library.h>
#include <memory>
#include <reader/tagged_block_reader.h>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

std::unordered_map<std::string, std::shared_ptr<SceneTree> > globalSceneTreeMap;

std::string generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4"; // UUID version 4
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen); // variant bits
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    return ss.str();
}

std::string addSceneTree(std::shared_ptr<SceneTree> tree) {
    std::string uuid = generateUUID();

    // Check if the UUID already exists
    int attempts = 0;
    while (globalSceneTreeMap.find(uuid) != globalSceneTreeMap.end()) {
        uuid = generateUUID();
        if (++attempts > 10) {
            throw std::runtime_error("Failed to generate a unique UUID after 10 attempts");
        }
    }

    globalSceneTreeMap[uuid] = std::move(tree);
    return uuid;
}

std::shared_ptr<SceneTree> getSceneTree(const std::string &treeId) {
    if (auto it = globalSceneTreeMap.find(treeId); it != globalSceneTreeMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool removeSceneTree(const std::string &uuid) {
    return globalSceneTreeMap.erase(uuid) > 0;
}

std::shared_ptr<TaggedBlockReader> prepareReader(const int inputFD) {
    // Map input file
    struct stat fd_stat;
    if (fstat(inputFD, &fd_stat) == -1) {
        throw std::runtime_error(std::format("Invalid file descriptor {}", inputFD));
    }
    size_t input_size = getFileSize(inputFD);

    // Initialize reader
    std::shared_ptr<TaggedBlockReader> reader = std::make_shared<V6Reader>(inputFD, input_size);

    if (!reader->readHeader()) {
        reader = std::make_shared<V5Reader>(inputFD, input_size);
        if (!reader->readHeader()) {
            throw CannotReadHeaderException("Failed to handle v6 and v5");
        }
    }
    return reader;
}

// EXPORT bool compileTreeToJson(const int inputFD, const int outputFD) {
//
// }

EXPORT const char *buildTree(const int inputFD) {
    logDebug(std::format("run build tree {}", inputFD));
    std::shared_ptr<TaggedBlockReader> reader;
    try {
        reader = prepareReader(inputFD);
    } catch (const CannotReadHeaderException &e) {
        logError(e.what());
        return "";
    } catch (const std::exception &e) {
        logError(e.what());
        return "";
    }

    logDebug("make shared tree");

    const auto tree = std::make_shared<SceneTree>();

    try {
        logDebug("build tree");
        reader->buildTree(*tree);
    } catch (const std::exception &e) {
        logError(std::format("{}\nFailed to build tree: {}", getStackTrace(), e.what()));
        return "";
    }

    static std::string result;

    try {
        result = addSceneTree(tree);
    } catch (const std::exception &e) {
        logError(std::format("{}\nFailed to add tree to tree map: {}", getStackTrace(), e.what()));
        return "";
    }

    return result.c_str();
}

EXPORT bool convertToSvg(const char *treeId, const int outputFD) {
    const auto tree = getSceneTree(treeId);
    if (!tree) {
        logError("Invalid treeId provided");
        return false;
    }

    // Dummy SVG content (replace with real conversion later)
    const char *svg_content = "<svg xmlns=\"http://www.w3.org/2000/svg\"></svg>";
    size_t svg_size = strlen(svg_content);

    // Write SVG data
    write(outputFD, svg_content, svg_size);

    return true;
}
