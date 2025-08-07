#pragma once
#include <stb_image.h>

struct LoadedImageData
{
    stbi_uc* pixels = nullptr; 
    int width = 0;
    int height = 0;
    int channels = 0; 
    int original_channels = 0;
};
