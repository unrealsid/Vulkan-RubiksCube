
#include "Vk_RubiksCube.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <thread>

#include "Config.h"
#include "core/Engine.h"


//
// void loadModel(Init& init, RenderData& renderData)
// {
//     VkUtils::ModelLoaderUtils modelUtils;
//     modelUtils.load_obj(std::string(RESOURCE_PATH) + "/models/rubiks_cube/RubiksCubeTextures/rubiksCubeTexture.obj", renderData.outVertices, renderData.outIndices, renderData.primitiveMaterialIndices, renderData.material_params, renderData.textureInfo);
//
//     for (const auto& texture : renderData.textureInfo)
//     {
//         LoadedImageData imgData = VMA_ImageUtils::loadImageFromFile(texture.second.path);
//         VMA_ImageUtils::textures.push_back(VMA_ImageUtils::createAndUploadImage(init, renderData, imgData));
//     }
//     
//     std::cout << "Vertices: " << renderData.outVertices.size() << std::endl;
//     std::cout << "Indices: " << renderData.outIndices.size() << std::endl;
//
//
//     // data.outVertices = {
//     //     // Front face (Z = +0.5)
//     //     {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 0: Bottom-left-front
//     //     {{ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 1: Bottom-right-front
//     //     {{ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 2: Top-right-front
//     //     {{-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 3: Top-left-front
//     //
//     //     // Back face (Z = -0.5)
//     //     {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 4: Bottom-left-back
//     //     {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 5: Bottom-right-back
//     //     {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 6: Top-right-back
//     //     {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}  // 7: Top-left-back
//     // };
//     //
//     // // Define index data (36 indices for 12 triangles)
//     // // This defines the triangles using the vertices above.
//     // // Assuming uint32_t for indices, change to uint16_t if you use that.
//     // data.outIndices = {
//     //     // Front face
//     //     0, 1, 2, 0, 2, 3,
//     //     // Back face
//     //     4, 6, 5, 4, 7, 6,
//     //     // Top face
//     //     3, 2, 6, 3, 6, 7,
//     //     // Bottom face
//     //     0, 4, 5, 0, 5, 1,
//     //     // Right face
//     //     1, 5, 6, 1, 6, 2,
//     //     // Left face
//     //     4, 0, 3, 4, 3, 7
//     // };
// }

int main()
{
    core::Engine& engine = core::Engine::get_instance();
    engine.init();
    engine.run();
    engine.cleanup();
    
    return 0;
}
