#ifndef RM_LINES_H
#define RM_LINES_H

#include "library.h"
#include "common/scene_tree.h"
#include <unordered_map>
#include <memory>

extern std::unordered_map<std::string, std::shared_ptr<SceneTree> > globalSceneTreeMap;

EXPORT const char *buildTree(int inputFD);

EXPORT bool convertToSvg(const char *treeId, int outputFD);
EXPORT bool convertToJson(const char *treeId, int outputFD);

#endif //RM_LINES_H
