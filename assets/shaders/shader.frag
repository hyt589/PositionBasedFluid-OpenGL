#version 410 core

out vec4 FragColor;
in vec2 uv;
in vec3 pos;
in vec3 normal;
in vec4 lightSpaceFragPos[4];

uniform sampler2D albedoMap;
uniform sampler2D normalMap;

//The metalness values are sampled from the B channel. The roughness values are sampled from the G channel
uniform sampler2D metallicRoughnessMap;
// no aoMap in this model
uniform sampler2D aoMap;

uniform float far_plane;

uniform int numLights;

#define MAX_LIGHT 4

uniform vec3 lightPos[MAX_LIGHT];
uniform vec3 lightColor[MAX_LIGHT];

uniform vec3 camPos;

const float PI = 3.14159265359;

uniform samplerCube depthCubeMap[MAX_LIGHT];


vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, uv).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(pos);
    vec3 Q2  = dFdy(pos);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N   = normalize(normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

bool isInShadow(vec3 fragPos, int i){
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos[i];
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(depthCubeMap[i], fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float bias = 0.0005; 

    return currentDepth - bias >= closestDepth;
}

vec3 visualizeShadow(vec3 fragPos, int i){
    vec3 fragToLight = fragPos - lightPos[i];
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(depthCubeMap[i], fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    return vec3(closestDepth);
}

void main(){

    vec3 albedo = pow(texture(albedoMap, uv).rgb, vec3(2.2));
    float metallic = texture(metallicRoughnessMap, uv).b;
    float roughness = texture(metallicRoughnessMap, uv).g;
    float ao = 1;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - pos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < numLights; i++){
        if(isInShadow(pos, i)){
            continue;
        }
        vec3 L = normalize(lightPos[i] - pos);
        vec3 H = normalize(V + L);
        float distance = length(lightPos[i] - pos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = 10 * lightColor[i] * attenuation;

        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = nominator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        // Lo += visualizeShadow(pos, i);
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    // FragColor = vec4(visualizeShadow(pos,0), 1.0);
    FragColor = vec4(color, 1.0);
}

