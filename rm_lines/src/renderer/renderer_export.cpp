#include <renderer/renderer_export.h>
#include <scene_tree/scene_tree_export.h>
#include <format>
#include <fstream>
#include <sstream>
#include <optional>
#include <mutex>

static std::mutex globalRendererMutex;
std::unordered_map<std::string, std::shared_ptr<Renderer> > globalRendererMap;

std::string addRenderer(std::shared_ptr<Renderer> renderer) {
    std::lock_guard<std::mutex> lock(globalRendererMutex);
    std::string uuid = generateUUID();

    // Check if the UUID already exists
    int attempts = 0;
    while (globalRendererMap.contains(uuid)) {
        uuid = generateUUID();
        if (++attempts > 10) {
            throw std::runtime_error("Failed to generate a unique UUID after 10 attempts");
        }
    }

    globalRendererMap[uuid] = std::move(renderer);
    return uuid;
}

std::shared_ptr<Renderer> getRenderer(const std::string &treeId) {
    std::lock_guard<std::mutex> lock(globalRendererMutex);
    auto it = globalRendererMap.find(treeId);
    if (it == globalRendererMap.end()) {
        return nullptr;
    }
    return it->second;
}

bool removeRenderer(const std::string &uuid) {
    std::lock_guard<std::mutex> lock(globalRendererMutex);
    return globalRendererMap.erase(uuid) > 0;
}

EXPORT const char *makeRenderer(const char *treeId, const int pageType, const bool landscape) {
    const auto tree = getSceneTree(treeId);

    std::shared_ptr<Renderer> renderer;

    try {
        renderer = std::make_shared<Renderer>(tree.get(), static_cast<PageType>(pageType), landscape);
    } catch (const std::exception &e) {
        logError(std::format("{}\nFailed to create renderer: {}", getStackTrace(), e.what()));
        return "";
    }

    thread_local std::string result;

    try {
        result = addRenderer(renderer);
    } catch (const std::exception &e) {
        logError(std::format("{}\nFailed to add tree to tree map: {}", getStackTrace(), e.what()));
        return "";
    }

    return result.c_str();
};

EXPORT int destroyRenderer(const char *rendererId) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return -1;
    }
    if (removeRenderer(rendererId)) {
        return sizeof(*renderer);
    }

    logError("Failed to remove renderer from renderer map");
    return -1;
}

EXPORT const char *getParagraphs(const char *rendererId) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return "";
    }
    const json j = renderer->getParagraphs();

    thread_local std::string result;
    result = j.dump();

    return result.c_str();
}

const char *getAnchors(const char *rendererId) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return "";
    }
    const json j = renderer->getAnchors();

    thread_local std::string result;
    result = j.dump();

    return result.c_str();
}

const char *getLayers(const char *rendererId) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return "";
    }
    const json j = renderer->getLayers();

    thread_local std::string result;
    result = j.dump();

    return result.c_str();
}

bool textToMdFile(const char *rendererId, const char *outputFile) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return false;
    }

    std::ofstream fileStream(outputFile);
    if (!fileStream) {
        logError(std::format("Failed to open file: {}", outputFile));
        return false;
    }
    renderer->toMd(fileStream);
    return true;
}


const char *textToMd(const char *rendererId) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return "";
    }

    thread_local std::string result;
    std::ostringstream stringStream;

    renderer->toMd(stringStream);
    result = stringStream.str();

    return result.c_str();
}

bool textToTxtFile(const char *rendererId, const char *outputFile) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return false;
    }

    std::ofstream fileStream(outputFile);
    if (!fileStream) {
        logError(std::format("Failed to open file: {}", outputFile));
        return false;
    }
    renderer->toTxt(fileStream);
    return true;
}


const char *textToTxt(const char *rendererId) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return "";
    }

    thread_local std::string result;
    std::ostringstream stringStream;

    renderer->toTxt(stringStream);
    result = stringStream.str();

    return result.c_str();
}

bool textToHtmlFile(const char *rendererId, const char *outputFile) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return false;
    }

    std::ofstream fileStream(outputFile);
    if (!fileStream) {
        logError(std::format("Failed to open file: {}", outputFile));
        return false;
    }
    renderer->toHtml(fileStream);
    return true;
}


const char *textToHtml(const char *rendererId) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return "";
    }

    thread_local std::string result;
    std::ostringstream stringStream;

    renderer->toHtml(stringStream);
    result = stringStream.str();

    return result.c_str();
}

void getFrame(
    const char *rendererId, uint32_t *data, const size_t dataSize,
    const int x, const int y,
    const int frameWidth, const int frameHeight,
    const int width, const int height, const bool antialias) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return;
    }
    renderer->getFrame(
        data, dataSize,
        Vector(x, y),
        Vector(frameWidth, frameHeight),
        Vector(width, height),
        antialias
    );
}

void setTemplate(
    const char *rendererId, const char *templateName) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return;
    }
    renderer->setTemplate(templateName);
}

const char *getSizeTracker(const char *rendererId, const char *stringlayerId) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return "";
    }
    const CrdtId layerId(stringlayerId);
    const json j = renderer->getSizeTracker(layerId)->toJson();

    thread_local std::string result;
    result = j.dump();

    return result.c_str();
}

void addImage(const char *rendererId, const char *uuid, const char *path) {
    const auto renderer = getRenderer(rendererId);
    if (!renderer) {
        logError("Invalid treeId provided");
        return;
    }
    renderer->addImage(uuid, path);
}
