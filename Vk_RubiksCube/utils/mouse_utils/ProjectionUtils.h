#pragma once
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/ext/quaternion_common.hpp>
#include "../../structs/EngineContext.h"

namespace utils
{
    class ProjectionUtils
    {
    public:
        //Returns world position of mouse click
        static glm::vec3 unproject_point(const EngineContext& engine_context, float depth_value, const glm::mat4& view_matrix, const glm::mat4& projection_matrix);
    };
}