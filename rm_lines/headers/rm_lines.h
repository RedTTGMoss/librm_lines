#ifndef RM_LINES_H
#define RM_LINES_H

#include "library.h"
#include "common/scene_tree.h"
#include <unordered_map>
#include <memory>

extern std::unordered_map<std::string, std::shared_ptr<SceneTree> > globalSceneTreeMap;

EXPORT const char *buildTree(const char *rmPath);

EXPORT bool convertToJson(const char *treeId, const char *outPath);

std::string generateUUID();
std::shared_ptr<SceneTree> getSceneTree(const std::string &treeId);

#endif //RM_LINES_H
