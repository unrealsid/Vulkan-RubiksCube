#version 460

#include "/include/mesh_shader_common.glsl"

layout(early_fragment_tests) in;

layout(location = 0) in vec3 inColor; 
layout(location = 1) in vec2 inUV;
layout(location = 2) flat in uint inMaterialIndex;
layout(location = 3) flat in uint inTexIndex;

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D textures[];

vec2 transformUV(vec2 uv, uint texIndex) 
{
    // Hardcoded transformation parameters
    // Scale, translation, and rotation for each texture index
    vec2 scale = vec2(-5.0, 5.0);
    vec2 translate = vec2(0.0, 0.0);
    float rotation = -3.14 / 2.0;

    // First center the UV for rotation and scaling
    vec2 centeredUV = uv - vec2(0.5, 0.5);

    // Apply rotation
    float cosAngle = cos(rotation);
    float sinAngle = sin(rotation);
    vec2 rotatedUV = vec2(
    centeredUV.x * cosAngle - centeredUV.y * sinAngle,
    centeredUV.x * sinAngle + centeredUV.y * cosAngle
    );

    // Apply scaling
    vec2 scaledUV = rotatedUV * scale;

    // Move back from center and apply translation
    return scaledUV + vec2(0.5, 0.5) + translate;
}

void main() 
{
    Material materialRef = pushConstants.materialsDataReference.materialParams[inMaterialIndex];

    // Now access the material properties
    vec4 diffuseColor = materialRef.diffuse;
    vec4 emissiveColor = materialRef.emissive;
    float shininess = materialRef.shininess.x;
    float alpha = materialRef.alpha.x;

    vec2 uv = transformUV(inUV, 0);
    vec4 texColor = texture(textures[nonuniformEXT(inTexIndex)], uv);
    vec4 finalColor = vec4(0.0);

    if (all(equal(texColor, vec4(0.0)))) 
    {
        finalColor = diffuseColor;
    }
    else
    {
        finalColor = texColor * diffuseColor;
    }
    
    //finalColor.rgb += emissiveColor.rgb;

    // Set alpha from material if needed, or you can use the texture's alpha
    finalColor.a = 1.0;

    outColor = finalColor;
}