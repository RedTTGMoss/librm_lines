#include "renderer/image_ref.h"
#include "../../headers/stb/stb_image.h"

ImageRef ImageRef::load(const char *fileName) {
    ImageRef imageRef;
    imageRef.data = stbi_load(fileName, &imageRef.w, &imageRef.h, &imageRef.channels, 4);
    return imageRef;
}
