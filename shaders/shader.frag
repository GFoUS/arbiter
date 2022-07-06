#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform frame_t {
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
} frame;

layout(set = 1, binding = 0) uniform material_t {
    vec4 baseColorFactor;
} material;
layout(set = 1, binding = 1) uniform sampler2D baseColorTexture;
layout(set = 1, binding = 2) uniform sampler2D normalTexture;

float specularStrength = 0.1;
float ambientStrength = 0.1;

void main() {
    float lightIntensity = 3.0;
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 light = lightColor * lightIntensity;
    vec3 lightPosition = vec3(0.0, 0.0, 0.0);

    vec3 normal = normalize(fragNormal);
    vec3 lightDirection = normalize(lightPosition - fragPosition); 

    // Ambient
    vec3 ambient = ambientStrength * light;

    // Diffuse
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diff * light;

    // Specular
    vec3 viewDirection = normalize(frame.cameraPosition - fragPosition);
    vec3 reflectDirection = reflect(-lightDirection, normal);  
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
    vec3 specular = specularStrength * spec * light; 

    vec4 color = material.baseColorFactor * texture(baseColorTexture, fragUV);
    outColor = vec4(ambient + diffuse, 1.0) * color;
}