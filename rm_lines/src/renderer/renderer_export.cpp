#include <renderer/renderer_export.h>
#include <scene_tree/scene_tree_export.h>
#include <format>
#include <fstream>

std::unordered_map<std::string, std::shared_ptr<Renderer> > globalRendererMap;

std::string addRenderer(std::shared_ptr<Renderer> renderer) {
  std::string uuid = generateUUID();

  // Check if the UUID already exists
  int attempts = 0;
  while (globalRendererMap.find(uuid) != globalRendererMap.end()) {
    uuid = generateUUID();
    if (++attempts > 10) {
      throw std::runtime_error("Failed to generate a unique UUID after 10 attempts");
    }
  }

  globalRendererMap[uuid] = std::move(renderer);
  return uuid;
}

std::shared_ptr<Renderer> getRenderer(const std::string &treeId) {
  if (auto it = globalRendererMap.find(treeId); it != globalRendererMap.end()) {
    return it->second;
  }
  return nullptr;
}

bool removeRenderer(const std::string &uuid) {
  return globalRendererMap.erase(uuid) > 0;
}

EXPORT const char *makeRenderer(const char *treeId, const int pageType, const bool landscape) {
  auto tree = getSceneTree(treeId);
  auto renderer = std::make_shared<Renderer>(tree.get(), static_cast<PageType>(pageType), landscape);

  static std::string result;

  try {
    result = addRenderer(renderer);
  } catch (const std::exception &e) {
    logError(std::format("{}\nFailed to add tree to tree map: {}", getStackTrace(), e.what()));
    return "";
  }

  return result.c_str();
};

EXPORT int destroyRenderer(const char *rendererId) {
  auto renderer = getRenderer(rendererId);
  if (!renderer) {
    logError("Invalid treeId provided");
    return -1;
  }
  int size = sizeof(*renderer);
  if (removeRenderer(rendererId)) {
    return size;
  }

  logError("Failed to remove renderer from renderer map");
  return -1;
}

EXPORT const char *getParagraphs(const char *rendererId) {
  auto renderer = getRenderer(rendererId);
  if (!renderer) {
    logError("Invalid treeId provided");
    return "";
  }
  json j = renderer->getParagraphs();

  static std::string result;
  result = j.dump();

  return result.c_str();
}

const char * textToMd(const char *rendererId) {
  const auto renderer = getRenderer(rendererId);
  if (!renderer) {
    logError("Invalid treeId provided");
    return "";
  }

  static std::string result;
  std::ostringstream stringStream;

  renderer->toMd(stringStream);
  result = stringStream.str();

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
