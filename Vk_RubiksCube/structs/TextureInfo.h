#pragma once
#include <string>

struct TextureInfo
{
    std::string path;
    enum class Type { Diffuse, Specular, Normal, Unknown } type;
        
    TextureInfo() : type(Type::Unknown) {}
    TextureInfo(std::string p, Type t) : path(std::move(p)), type(t) {}
};
