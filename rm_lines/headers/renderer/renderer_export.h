#pragma once

#include <renderer/renderer.h>

extern std::unordered_map<std::string, std::shared_ptr<Renderer> > globalRendererMap;

std::shared_ptr<Renderer> getRenderer(const std::string &treeId);


EXPORT const char *makeRenderer(const char *treeId, int pageType, bool landscape);

EXPORT int destroyRenderer(const char *rendererId);

EXPORT const char *getParagraphs(const char *rendererId);

EXPORT const char *getLayers(const char *rendererId);

EXPORT bool textToMdFile(const char *rendererId, const char *outputFile);

EXPORT const char *textToMd(const char *rendererId);

EXPORT bool textToTxtFile(const char *rendererId, const char *outputFile);

EXPORT const char *textToTxt(const char *rendererId);

EXPORT bool textToHtmlFile(const char *rendererId, const char *outputFile);

EXPORT const char *textToHtml(const char *rendererId);

EXPORT void getFrame(const char *rendererId, uint32_t *data, size_t dataSize, int x, int y, int width,
                     int height,
                     float scale);
