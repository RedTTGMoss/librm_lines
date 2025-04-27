#include "../../headers/scene_tree/scene_tree_export.h"
#include "v5/reader.h"
#include "v6/reader.h"
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <format>
#include <library.h>
#include <memory>
#include <reader/tagged_block_reader.h>
#include <nlohmann/json.hpp>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

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

std::shared_ptr<TaggedBlockReader> prepareReader(FILE *inputFile) {
    // Get size of input file
    size_t inputSize = getFileSize(inputFile);

    // Initialize reader
    std::shared_ptr<TaggedBlockReader> reader = std::make_shared<V6Reader>(inputFile, inputSize);

    if (!reader->readHeader()) {
        reader = std::make_shared<V5Reader>(inputFile, inputSize);
        if (!reader->readHeader()) {
            throw CannotReadHeaderException("Failed to handle v6 and v5");
        }
    }
    return reader;
}

EXPORT bool convertToJson(const char *treeId, const char *outPath) {
    const auto tree = getSceneTree(treeId);
    if (!tree) {
        logError("Invalid treeId provided");
        return false;
    }

    const json j = tree->toJson();

    const auto jsonString = j.dump();

    FILE *outputFile = fopen(outPath, "wb");
    if (!outputFile) {
        logError("Could not open output file");
        return false;
    }
    fwrite(jsonString.c_str(), jsonString.size(), 1, outputFile);
    fclose(outputFile);

    return true;
}

EXPORT const char * getSceneInfo(const char *treeId) {
    const auto tree = getSceneTree(treeId);
    if (!tree) {
        logError("Invalid treeId provided");
        return "";
    }

    if (!tree->sceneInfo) {
        return "";
    }

    const json j = tree->sceneInfo.value().toJson();

    static std::string result;
    result = j.dump();

    return result.c_str();
}

EXPORT const char *buildTree(const char *rmPath) {
    std::shared_ptr<TaggedBlockReader> reader;
    FILE *inputFile = std::fopen(rmPath, "rb");
    if (!inputFile) {
        logError("Could not open file");
        return "";
    }
    try {
        reader = prepareReader(inputFile);
    } catch (const CannotReadHeaderException &e) {
        logError(e.what());
        fclose(inputFile);
        return "";
    } catch (const std::exception &e) {
        logError(e.what());
        fclose(inputFile);
        return "";
    }


    const auto tree = std::make_shared<SceneTree>();

    try {
        reader->buildTree(*tree);
    } catch (const std::exception &e) {
        logError(std::format("{}\nFailed to build tree: {}", getStackTrace(), e.what()));
        fclose(inputFile);
        return "";
    }

    static std::string result;

    try {
        result = addSceneTree(tree);
    } catch (const std::exception &e) {
        logError(std::format("{}\nFailed to add tree to tree map: {}", getStackTrace(), e.what()));
        fclose(inputFile);
        return "";
    }

    fclose(inputFile);
    return result.c_str();
}

EXPORT int destroyTree(const char *treeId) {
    auto tree = getSceneTree(treeId);
    if (!tree) {
        logError("Invalid treeId provided");
        return -1;
    }
    int size = sizeof(*tree);
    if (removeSceneTree(treeId)) {
        return size;
    }

    logError("Failed to remove tree from tree map");
    return -1;
}