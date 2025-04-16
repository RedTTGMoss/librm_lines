#include <renderer/renderer_export.h>
#include <rm_lines.h>
#include <format>

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

EXPORT const char *makeRenderer(const char *treeId) {
  auto tree = getSceneTree(treeId);
  auto renderer = std::make_shared<Renderer>(tree.get());

  static std::string result;

  try {
    result = addRenderer(renderer);
  } catch (const std::exception &e) {
    logError(std::format("{}\nFailed to add tree to tree map: {}", getStackTrace(), e.what()));
    return "";
  }

  return result.c_str();
};