#include <iostream>
#include <filesystem>
#include <fstream>
#include <queue>

#include "library.h"
#include "renderer/renderer_export.h"
#include "scene_tree/scene_tree_export.h"
namespace fs = std::filesystem;

#define DIR_OUT "./output/"
#define HTML_OUT "./output/html/"
#define JSON_OUT "./output/json/"
#define MD_OUT "./output/md/"
#define SVG_OUT "./output/svg/"
#define PNG_OUT "./output/png/"
#define PARA_OUT "./output/paragraphs/"
#define LAYERS_OUT "./output/layers/"
#define RAW_TEXT_OUT "./output/raw_text/"
#define TEXT_EXPAND_PYTHON_OUT "./output/text_expand/"
#define ANCHOR_TEST_PYTHON_OUT "./output/anchor_test/"

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

void replaceNewLine(std::string &string) {
    size_t pos = 0;
    while ((pos = string.find("\n", pos)) != std::string::npos) {
        string.replace(pos, 1, "\\n");
        pos += 2; // Move past the replaced "\\n"
    }
}

std::string safeString(std::string input) {
    std::string output;
    for (const char c: input) {
        if (c == '\"') {
            output += "\\\"";
        } else if (c == '\'') {
            output += "\\\'";
        } else if (c == '\\') {
            output += "\\\\";
        } else if (c == '\n') {
            output += "\\n";
        } else {
            output += c;
        }
    }
    return output;
}

bool processFile(const std::string &filename, const std::string &path) {
    logMessage(std::format("\nProcessing page {}", filename));

    const char *treeId = buildTree(path.c_str());
    if (!treeId) {
        logError(std::format("Failed to build tree for file: {}", filename));
        return false;
    }
    auto tree = getSceneTree(treeId).get();
    auto textCopy = tree->rootText;
    auto renderer = Renderer(tree, NOTEBOOK, false);

    // Checking text
    // Initialize both tracking ids to the END_MARKER for compatibility
    CrdtId oldId = END_MARKER;
    CrdtId currentId = END_MARKER;
    // Iterate over the paragraphs and their contents
    for (const auto &paragraph: renderer.textDocument.paragraphs) {
        for (const auto &string: paragraph.contents) {
            for (const auto &characterId: string.characterIDs) {
                auto currentTextItem = renderer.textDocument.text.items[currentId];
                if (
                    getTextItemContents(currentTextItem) != "\n" &&
                    currentId != END_MARKER &&
                    characterId != currentId
                ) {
                    auto oldTextItem = renderer.textDocument.text.items[oldId];

                    logError(std::format("Character ID mismatch in file {}: expected {}, got {}", filename,
                                         currentId.repr(), characterId.repr()));
                    logError(std::format(
                        "-> According to the current element ({}) the previous element should be {} but it is actually {}",
                        currentId.repr(), currentTextItem.leftId.repr(), oldId.repr()));
                    logError(std::format(
                        "-> According to the previous element ({}) this it's neighbours should be {} <-{}-> {}",
                        oldId.repr(), oldTextItem.leftId.repr(), oldId.repr(), oldTextItem.rightId.repr()));
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

    // Run exports
    std::string htmlFile = HTML_OUT + filename + ".html";
    std::string jsonFile = JSON_OUT + filename + ".json";
    std::string mdFile = MD_OUT + filename + ".md";
    std::string svgFile = SVG_OUT + filename + ".svg";
    std::string pngFile = PNG_OUT + filename + ".png";
    std::string paraFile = PARA_OUT + filename + ".json";
    std::string layersFile = LAYERS_OUT + filename + ".json";
    std::string rawTextFile = RAW_TEXT_OUT + filename + ".bin";
    std::string textExpandPythonFile = TEXT_EXPAND_PYTHON_OUT + filename + ".py";
    std::string anchorTestPythonFile = ANCHOR_TEST_PYTHON_OUT + filename + ".json";

    convertToJsonFile(treeId, jsonFile.c_str());

    std::ofstream htmlFilePtr(htmlFile.c_str());
    std::ofstream mdFilePtr(mdFile.c_str());
    std::ofstream svgFilePtr(svgFile.c_str());
    std::ofstream pngFilePtr(pngFile.c_str());
    std::ofstream paraFilePtr(paraFile.c_str());
    std::ofstream layersFilePtr(layersFile.c_str());
    std::ofstream rawTextFilePtr(rawTextFile.c_str());
    std::ofstream textExpandPythonFilePtr(textExpandPythonFile.c_str());
    std::ofstream anchorTestPythonFilePtr(anchorTestPythonFile.c_str());
    if (!htmlFilePtr || !mdFilePtr || !svgFilePtr || !pngFilePtr || !paraFilePtr || !layersFilePtr || !rawTextFilePtr ||
        !textExpandPythonFilePtr || !anchorTestPythonFilePtr) {
        logError(std::format("Failed to open output files for page \"{}\"", filename));
        destroyTree(treeId);
        return false;
    }
    try {
        renderer.toHtml(htmlFilePtr);
        renderer.toMd(mdFilePtr);
        json paragraphs = renderer.getParagraphs();
        paraFilePtr << paragraphs.dump(4);
        json layers = renderer.getLayers();
        layersFilePtr << layers.dump(4);
        json anchorTestData = json::array();
        for (const auto &layer: renderer.layers) {
            json layerJ;
            layerJ["layerId"] = layer.groupId.toJson();
            layerJ["label"] = layer.getLabel();
            layerJ["visible"] = layer.visible;
            layerJ["lines"] = json::array();
            for (const auto &[line, groupId, offsetX, offsetY]: layer.lines) {
                json lineJ;
                lineJ["offsetX"] = offsetX;
                lineJ["offsetY"] = offsetY;
                lineJ["groupId"] = groupId.toJson();
                lineJ["value"] = line.toJson();
                layerJ["lines"].push_back(lineJ);
            }
            anchorTestData.push_back(layerJ);
        }
        anchorTestPythonFilePtr << anchorTestData.dump(4);

        // Export the raw characters
        for (const auto &id: renderer.textDocument.text.items.getSortedIds()) {
            if (auto item = renderer.textDocument.text.items[id]; item.value.has_value()) {
                if (std::holds_alternative<std::string>(item.value.value())) {
                    rawTextFilePtr << std::get<std::string>(item.value.value());
                }
            }
        }

        if (textCopy.has_value()) {
            // Export the original text items in raw
            rawTextFilePtr << "\n\n";
            for (const auto &value: textCopy.value().items.sequence | std::views::values) {
                if (value.value.has_value()) {
                    if (std::holds_alternative<std::string>(value.value.value())) {
                        auto string = std::get<std::string>(value.value.value());
                        replaceNewLine(string);
                        rawTextFilePtr << ">> " << string << "\n";
                    }
                }
            }

            // Export the original text items in json
            rawTextFilePtr << "\n\n" << textCopy.value().items.toJson().dump(4);

            // Export the text to python symbols for testing
            textExpandPythonFilePtr << "[";
            for (const auto &id: renderer.textDocument.text.items.getSortedIds()) {
                if (auto item = renderer.textDocument.text.items[id]; item.value.has_value()) {
                    if (std::holds_alternative<std::string>(item.value.value())) {
                        auto string = safeString(std::get<std::string>(item.value.value()));
                        textExpandPythonFilePtr << std::format(
                            "CrdtSequenceItem("
                            "item_id=CrdtId({}, {}),"
                            "left_id=CrdtId({}, {}),"
                            "right_id=CrdtId({}, {}),"
                            "deleted_length={},value=\"{}\""
                            "),",
                            item.itemId.first, item.itemId.second,
                            item.leftId.first, item.leftId.second,
                            item.rightId.first, item.rightId.second,
                            item.deletedLength, string
                        );
                    } else if (std::holds_alternative<uint32_t>(item.value.value())) {
                        textExpandPythonFilePtr << std::format(
                            "CrdtSequenceItem("
                            "item_id=CrdtId({}, {}),"
                            "left_id=CrdtId({}, {}),"
                            "right_id=CrdtId({}, {}),"
                            "deleted_length={},value={}"
                            "),",
                            item.itemId.first, item.itemId.second,
                            item.leftId.first, item.leftId.second,
                            item.rightId.first, item.rightId.second,
                            item.deletedLength,
                            std::get<std::uint32_t>(item.value.value())
                        );
                    }
                }
            }
            textExpandPythonFilePtr << "]";
        }
    } catch (const std::exception &e) {
        logError(std::format("Failed to export page \"{}\"", filename));
        logError(std::format("Exception: {}", e.what()));
        destroyTree(treeId);
        return false;
    }


    // Clean up
    destroyTree(treeId);
    return true;
}

void loggerDefault(const char *msg) {
    ;
    std::cout << msg << std::endl;
}

void loggerError(const char *msg) {
    std::cerr << msg << std::endl;
}

int main() {
    // Initialize the library
    setLogger(loggerDefault);
    setDebugLogger(loggerDefault);
    setErrorLogger(loggerError);

    // Ensure the location matches the source `tests/`
    if (!fs::exists("./test.cpp")) {
        logError("Please run this test from the tests directory.");
        return -1;
    }

    // Make sure the output directories exist
    fs::create_directories(DIR_OUT);
    fs::create_directories(HTML_OUT);
    fs::create_directories(JSON_OUT);
    fs::create_directories(MD_OUT);
    fs::create_directories(SVG_OUT);
    fs::create_directories(PNG_OUT);
    fs::create_directories(PARA_OUT);
    fs::create_directories(LAYERS_OUT);
    fs::create_directories(RAW_TEXT_OUT);
    fs::create_directories(TEXT_EXPAND_PYTHON_OUT);
    fs::create_directories(ANCHOR_TEST_PYTHON_OUT);

    try {
        // Set the directory path

        // Iterate over the directory entries
        for (const fs::path dirPath = "./files"; const auto &entry: fs::directory_iterator(dirPath)) {
            std::string filename = entry.path().filename().string();
            std::string file = entry.path().string();
            if (!file.ends_with(".rm")) {
                std::cerr << "File " << file << " is not a LINES file" << std::endl;
                return -1;
            }
            if (!processFile(filename.substr(0, filename.length() - 3), file))
                return -1;
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    return 0;
}
