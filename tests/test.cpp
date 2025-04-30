#include <iostream>
#include <filesystem>
#include <queue>

#include "library.h"
#include "renderer/renderer_export.h"
#include "scene_tree/scene_tree_export.h"
namespace fs = std::filesystem;

std::string getTextItemContents(const TextItem item) {
    if (!item.value.has_value()) {
        return "";
    }
    if (std::holds_alternative<std::string>(item.value.value())) {
        return std::get<std::string>(item.value.value());
    }
    if (std::holds_alternative<uint32_t>(item.value.value())) {
        return std::format("Formatting ({})", std::get<uint32_t>(item.value.value()));
    }
}

bool testFile(const std::string &filename, const std::string &path) {
    logMessage(std::format("Processing page {}", filename));

    const char *treeId = buildTree(path.c_str());
    if (!treeId) {
        logError(std::format("Failed to build tree for file: {}", filename));
        return false;
    }
    auto tree = getSceneTree(treeId).get();
    auto renderer = Renderer(tree, NOTEBOOK, false);

    // Checking text
    // Initialize both tracking ids to the END_MARKER for compatibility
    CrdtId oldId = END_MARKER;
    CrdtId currentId = END_MARKER;
    // Iterate over the paragraphs and their contents
    for (const auto &paragraph : renderer.textDocument.paragraphs) {
        for (const auto &string : paragraph.contents) {
            for (const auto &characterId : string.characterIDs) {
                auto currentTextItem = renderer.textDocument.text.items[currentId];
                if (
                    getTextItemContents(currentTextItem) != "\n" &&
                    currentId != END_MARKER &&
                    characterId != currentId
                    ) {

                    auto oldTextItem = renderer.textDocument.text.items[oldId];

                    logError(std::format("Character ID mismatch in file {}: expected {}, got {}", filename, currentId.repr(), characterId.repr()));
                    logError(std::format("-> According to the current element ({}) the previous element should be {} but it is actually {}", currentId.repr(), currentTextItem.leftId.repr(), oldId.repr()));
                    logError(std::format("-> According to the previous element ({}) this it's neighbours should be {} <-{}-> {}", oldId.repr(), oldTextItem.leftId.repr(), oldId.repr(), oldTextItem.rightId.repr()));
                    logError(std::format("-> The current item contains: {}", getTextItemContents(currentTextItem)));
                    logError(std::format("-> The previous item contains: {}", getTextItemContents(oldTextItem)));
                    destroyTree(treeId);
                    return false;
                }
                oldId = characterId;
                currentId = currentTextItem.rightId; // Expect the next character to be the rightId of the current one
            }
        }
    }

    // Clean up
    destroyTree(treeId);
    return true;
}

void loggerDefault(const char * msg) {;
    std::cout << msg << std::endl;
}

void loggerError(const char * msg) {
    std::cerr << msg << std::endl;
}

int main() {
    // Initialize the library
    setLogger(loggerDefault);
    setDebugLogger(loggerDefault);
    setErrorLogger(loggerError);

    try {
        // Set the directory path

        // Iterate over the directory entries
        for (const fs::path dirPath = "./files"; const auto& entry : fs::directory_iterator(dirPath)) {
            std::string filename = entry.path().filename().string();
            std::string file = entry.path().string();
            if (!file.ends_with(".rm")) {
                std::cerr << "File " << file << " is not a LINES file" << std::endl;
                return -1;
            }
            if (!testFile(filename.substr(0, filename.length()-3), file))
                return -1;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    return 0;
}
