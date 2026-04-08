#include <iostream>
#include <filesystem>
#include <fstream>
#include <queue>

#include "library.h"
#include "renderer/renderer_export.h"
#include "scene_tree/scene_tree_editor.h"
#include "scene_tree/scene_tree_export.h"
namespace fs = std::filesystem;

#define RM_OUT "./output/rm/"
#define JSON_OUT "./output/json/"
#define COLOR_RESET  "\033[0m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RED    "\033[31m"

void loggerDefault(const char *msg) {
    std::cout << msg << COLOR_RESET << std::endl;
}

void loggerDebug(const char *msg) {
    std::cout << COLOR_YELLOW << msg << COLOR_RESET << std::endl;
}

void loggerError(const char *msg) {
    std::cout << COLOR_RED << msg << COLOR_RESET << std::endl;
}

class File {
public:
    Renderer *renderer;
    SceneTreeEditor *tree;
    std::string name;

    explicit File(const std::string &name) : name(name) {
        tree = new SceneTreeEditor();
        tree->init();
        renderer = new Renderer(tree, NOTEBOOK, false);
    }

    ~File() {
        delete renderer;
        delete tree;
    }

    void save() {
        const std::string jsonFile = JSON_OUT + name + " - test write.json";
        const std::string rmFile = RM_OUT + name + " - test write.rm";
        std::ofstream rmFilePtr(rmFile.c_str());
        const json j = tree->toJson();
        const auto jsonString = j.dump();

        if (FILE *outputFile = fopen(jsonFile.c_str(), "wb")) {
            fwrite(jsonString.c_str(), jsonString.size(), 1, outputFile);
            fclose(outputFile);
        }

        renderer->toRM(rmFilePtr);
        rmFilePtr.close();
    }
};

int main(const int argc, char *argv[]) {
    // Initialize the library
    setLogger(loggerDefault);
    setDebugLogger(loggerDebug);
    setErrorLogger(loggerError);
    setDebugMode(true);
    // Ensure the location matches the source `tests/`
    if (!fs::exists("./test_writes.cpp")) {
        logError("Please run this test from the tests directory.");
        return -1;
    }

    fs::create_directories(RM_OUT);
    fs::create_directories(JSON_OUT);

    auto test_draw = File("Draw");
    test_draw.save();
}
