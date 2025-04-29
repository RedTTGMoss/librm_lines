#ifndef RENDERER_EXPORT_H
#define RENDERER_EXPORT_H

#include <renderer/renderer.h>

extern std::unordered_map<std::string, std::shared_ptr<Renderer> > globalRendererMap;

EXPORT const char *makeRenderer(const char *treeId, int pageType, bool landscape);

EXPORT int destroyRenderer(const char *rendererId);

EXPORT const char *getParagraphs(const char *rendererId);
EXPORT bool textToMdFile(const char *rendererId, const char *outputFile);
EXPORT const char *textToMd(const char *rendererId);
EXPORT bool textToHtmlFile(const char *rendererId, const char *outputFile);
EXPORT const char *textToHtml(const char *rendererId);

#endif //RENDERER_EXPORT_H
