#ifndef RM_LINES_H
#define RM_LINES_H

#include "../library.h"
#include "scene_tree.h"
#include <unordered_map>
#include <memory>

extern std::unordered_map<std::string, std::shared_ptr<SceneTree> > globalSceneTreeMap;

EXPORT const char *buildTree(const char *rmPath);
EXPORT int destroyTree(const char *treeId);

EXPORT bool convertToJsonFile(const char *treeId, const char *outPath);
EXPORT const char* convertToJson(const char *treeId);
EXPORT const char *getSceneInfo(const char *treeId);

std::string generateUUID();
std::shared_ptr<SceneTree> getSceneTree(const std::string &treeId);

#endif //RM_LINES_H
