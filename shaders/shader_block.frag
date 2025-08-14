#version 450

// Inputs.
layout(location = 0) in vec3 inFragColor;
layout(location = 1) in vec2 inFragTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inFragPos;

// Outputs.
layout(location = 0) out vec4 outColor;

// Uniforms.
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform PhongInfo
{
    vec3 lightDir; // Direction to light.
    vec3 lightColor;
    vec3 viewPos;
} lightingInfo;

void main()
{
    // Ambient.
    float ambient_strength = 0.1;
    vec3 ambient_val = ambient_strength * lightingInfo.lightColor;

    // Diffuse.
    vec3 normal = normalize(inNormal);
    //vec3 light_dir = normalize(lightingInfo.lightPos - fragPos);
    vec3 light_dir = normalize(lightingInfo.lightDir);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse_val = diff * lightingInfo.lightColor;

    // Specular.
    float spec_strength = 0.5;
    vec3 view_dir = normalize(lightingInfo.viewPos - inFragPos);
    vec3 reflect_dir = reflect(-light_dir, normal);  
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular_val = spec_strength * spec * lightingInfo.lightColor;  

    vec3 result = (ambient_val + diffuse_val + specular_val) * inFragColor * texture(texSampler, inFragTexCoord).rgb;
	outColor = vec4(result, 1.0);
}
