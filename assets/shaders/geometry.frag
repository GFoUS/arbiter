#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outMetallicRoughness;

layout(set = 0, binding = 0) uniform frame_t {
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
} frame;

layout(set = 1, binding = 0) uniform material_t {
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
} material;
layout(set = 1, binding = 1) uniform sampler2D baseColorTexture;
layout(set = 1, binding = 2) uniform sampler2D normalTexture;
layout(set = 1, binding = 3) uniform sampler2D metallicRoughnessTexture;

void main() {
    outAlbedo = material.baseColorFactor * texture(baseColorTexture, fragUV);
    outPosition = vec4(fragPosition, 1.0);
    outNormal = texture(normalTexture, fragUV);
    outMetallicRoughness = vec4(material.metallicFactor * texture(metallicRoughnessTexture, fragUV).b,
                                material.roughnessFactor * texture(metallicRoughnessTexture, fragUV).g, 0.0, 1.0);
}