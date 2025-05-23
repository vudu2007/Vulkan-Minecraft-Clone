#version 450

// Inputs.
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inInstancePos;

// Outputs.
layout(location = 0) out vec3 outFragColor;
layout(location = 1) out vec2 outfragTexCoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outFragPos;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos + inInstancePos, 1.0);
    
    outFragColor = inColor;
    outfragTexCoord = inTexCoord;
    outNormal = mat3(transpose(inverse(ubo.model))) * inNormal;  
    outFragPos = vec3(ubo.model * vec4(inPos + inInstancePos, 1.0));
}
