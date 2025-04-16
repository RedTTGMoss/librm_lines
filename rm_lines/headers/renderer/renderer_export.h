#ifndef RENDERER_EXPORT_H
#define RENDERER_EXPORT_H

#include <renderer/renderer.h>

extern std::unordered_map<std::string, std::shared_ptr<Renderer> > globalRendererMap;

EXPORT const char *makeRenderer(const char *treeId);

#endif //RENDERER_EXPORT_H
