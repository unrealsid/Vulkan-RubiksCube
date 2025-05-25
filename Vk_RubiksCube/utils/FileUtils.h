#pragma once
#include <cassert>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

class FileUtils
{
public:
    static void loadShader(std::string filename, char* &code, size_t &size);
};
