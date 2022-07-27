#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform frame_t {
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
} frame;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInputMS albedoTexture;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInputMS positionTexture;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInputMS normalTexture;
layout(input_attachment_index = 3, set = 1, binding = 3) uniform subpassInputMS metallicRoughnessTexture;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

void main() {
    vec4 albedo = subpassLoad(albedoTexture, gl_SampleID);
    vec3 position = subpassLoad(positionTexture, gl_SampleID).xyz;
    vec3 normal = normalize(subpassLoad(normalTexture, gl_SampleID)).xyz;
    vec2 metallicRoughness = subpassLoad(metallicRoughnessTexture, gl_SampleID).rg;
    float metallic = metallicRoughness.r;
    float roughness = metallicRoughness.g;

    vec3 lightPosition = vec3(100.0, 100.0, 100.0);
    vec3 lightColor = vec3(1000000, 1000000, 1000000);

    vec3 N = normalize(normal); 
    vec3 V = normalize(frame.cameraPosition - position);

    vec3 Lo = vec3(0.0);
    vec3 L = normalize(lightPosition - position);
    vec3 H = normalize(V + L);

    float distance    = length(lightPosition - position);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = lightColor * attenuation; 

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo.rgb, vec3(metallic));
    vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(N, H, roughness);       
    float G   = GeometrySmith(N, V, L, roughness);    

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0001;
    vec3 specular     = numerator / denominator;  

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    
    kD *= 1.0 - metallic;
  
    float NdotL = max(dot(N, L), 0.0);        
    Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.03) * albedo.rgb;
    outColor     = vec4(ambient + Lo, 1.0);  
}