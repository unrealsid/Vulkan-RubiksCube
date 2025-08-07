#include "FileUtils.h"

void utils::FileUtils::loadShader(std::string filename, char*& code, size_t& size)
{
    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);
    if (is.is_open())
    {
        size = is.tellg();
        is.seekg(0, std::ios::beg);
        code = new char[size];
        is.read(code, size);
        is.close();
        assert(size > 0);
    }
    else
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}
