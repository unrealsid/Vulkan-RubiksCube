#version 460
#extension GL_EXT_debug_printf : enable

// Input vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// Output to fragment shader
layout(location = 0) out vec3 outColor;

// --- Hardcoded Matrices ---
// (written column-major)

const mat4 model = mat4(
0.5, 0.0, 0.0, 0.0,   // X column
0.0, 0.5, 0.0, 0.0,   // Y column
0.0, 0.0, 0.5, 0.0,   // Z column
0.0, 0.0, 5.0, 1.0    // W column (translation here!)
);

const mat4 view = mat4(
1.0, 0.0, 0.0, 0.0,
0.0, 1.0, 0.0, 0.0,
0.0, 0.0, 1.0, 0.0,
0.0, 0.0, -2.0, 1.0
);

const float aspect = 1024.0 / 1024.0;
const float fov = radians(75.0);
const float near = 0.01;
const float far = 1000.0;

// Vulkan clip correction matrix (for Vulkan NDC)
const mat4 vulkanClipCorrection = mat4(
1.0,  0.0, 0.0, 0.0,
0.0, -1.0, 0.0, 0.0,
0.0,  0.0, 0.5, 0.0,
0.0,  0.0, 0.5, 1.0
);

void main()
{
    const mat4 proj = mat4(
    1.0 / (aspect * tan(fov / 2.0)), 0.0, 0.0, 0.0,  // Column 0
    0.0, 1.0 / tan(fov / 2.0), 0.0, 0.0,            // Column 1
    0.0, 0.0, -(far + near) / (far - near), -1.0,   // Column 2
    0.0, 0.0, -2.0 * far * near / (far - near), 0.0  // Column 3
    );
    
    mat4 correctedProj = vulkanClipCorrection * proj;
    gl_Position = vec4(inPosition, 1.0);

    debugPrintfEXT("Value of inPosition: %v3", inPosition);

    // Simple constant color output
    outColor = vec3(1.0, 1.0, 1.0);
}

/*#version 460

// Output to fragment shader
layout(location = 0) out vec3 outColor;

// Hardcoded triangle vertices
const vec3 positions[3] = vec3[](
vec3(-0.5, -0.5, 0.0),
vec3(0.5, -0.5, 0.0),
vec3(0.0, 0.5, 0.0)
);

// Hardcoded colors for each vertex
const vec3 colors[3] = vec3[](
vec3(1.0, 0.0, 0.0),
vec3(0.0, 1.0, 0.0),
vec3(0.0, 0.0, 1.0)
);

void main() {
    // Use gl_VertexIndex to get the current vertex
    vec3 pos = positions[gl_VertexIndex];

    // Simple pass-through transformation
    gl_Position = vec4(pos, 1.0);

    // Output color based on vertex
    outColor = colors[gl_VertexIndex];
}
*/