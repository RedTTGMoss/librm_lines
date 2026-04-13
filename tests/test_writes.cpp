#include <iostream>
#include <filesystem>
#include <fstream>
#include <queue>

#define STB_IMAGE_IMPLEMENTATION
#include "../rm_lines/headers/stb/stb_image.h"

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
using V = AdvancedMath::Vector;
using R = AdvancedMath::Rect;

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

    void drawCat() {
        const int width = tree->sceneInfo->paperSize->first;
        const int height = tree->sceneInfo->paperSize->second;

        int w, h, channels;
        int offset = 20;
        unsigned char *data = stbi_load("images/lines_icon.png", &w, &h, &channels, 4);

        logDebug(std::format("Paper size: {} x {}", width, height));
        const int offsetX = width / 2 - w - offset;
        const int offsetY = height - h - offset;
        auto line_row = [&](int y) {
            if (w <= 0 || h <= 0 || channels < 3) return;

            const int base_x = offsetX;
            const int draw_y = offsetY + y;

            int run_start_x = 0;
            int i = (y * w) * channels;

            Color current{data[i + 2], data[i + 1], data[i + 0], data[i + 3]};

            for (int x = 2; x < w; x += 2) {
                i = (y * w + x) * channels;

                const Color next{data[i + 2], data[i + 1], data[i + 0], data[i + 3]};

                if (next.red != current.red || next.green != current.green || next.blue != current.blue || next.alpha !=
                    current.alpha) {
                    tree->startLine()
                            .setPen(FINELINER_2)
                            .setRGBA(current)
                            .addPoint(base_x + run_start_x, draw_y)
                            .addPoint(base_x + x, draw_y)
                            .endLine();

                    current = next;
                    run_start_x = x;
                }
            }

            tree->startLine()
                    .setPen(FINELINER_2)
                    .setRGBA(current)
                    .addPoint(base_x + run_start_x, draw_y)
                    .addPoint(base_x + w, draw_y)
                    .endLine();
        };

        for (int y = 0; y < h; y += 2) {
            line_row(y);
        }
    }

    void addImage() {
        auto rainbow = tree->addImageInfo("rainbow.png");
        auto sillyCat = tree->addImageInfo("silly_cat.png");

        tree->addImage(rainbow, {
                           V{322.65, 1699.82},
                           V{560.82, 1499.97},
                           V{760.67, 1738.15},
                           V{522.50, 1938.00},
                       }
        );
        tree->addImage(sillyCat, {
                           V{-667.22, 5.42},
                           V{-368.84, 31.53},
                           V{-394.94, 329.91},
                           V{-693.32, 303.80},
                       }
        );
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

    auto testDrawCat = File("Cat");
    testDrawCat.drawCat();
    testDrawCat.save();

    auto testImage = File("Image");
    testImage.addImage();
    testImage.save();
}
