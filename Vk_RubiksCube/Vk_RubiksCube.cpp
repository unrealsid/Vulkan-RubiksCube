#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "VK_RubiksCube.h"

int main()
{
    VulkanRenderer renderer;

    try
    {
        renderer.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}