#version 450

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(set = 0, binding = 0) uniform frame_t {
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
} frame;

layout(set = 2, binding = 0) uniform model_t {
    mat4 model;
} model;

void main() {
    gl_Position = frame.proj * frame.view * model.model * vec4(inPosition, 1.0);
    fragPosition = inPosition;
    fragNormal = inNormal;
    fragUV = inUV;
}