#version 410 core

out vec4 FragColor;
in vec2 uv;
in vec3 pos;
in vec3 normal;

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

uniform int mode;

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

float VectorToDepthValue(vec3 Vec)
{
    vec3 AbsVec = abs(Vec);
    float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));

    const float f = 2048.0;
    const float n = 1.0;
    float NormZComp = (f+n) / (f-n) - (2*f*n)/(f-n)/LocalZcomp;
    return (NormZComp + 1.0) * 0.5;
}


void main(){

    vec3 albedo = pow(texture(albedoMap, uv).rgb, vec3(2.2));
    float metallic = texture(metallicRoughnessMap, uv).b;
    float roughness = texture(metallicRoughnessMap, uv).g;
    float ao = 1;
    float alpha = pow(texture(albedoMap, uv).rgba, vec4(2.2)).a;
    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - pos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < numLights; i++){
        
        vec3 L = normalize(lightPos[i] - pos);
        vec3 H = normalize(V + L);
        float r = length(lightPos[i] - pos);
        float attenuation = 1.0 / (r * r);
        vec3 radiance = 10 * lightColor[i] * attenuation;

        float depthComponent = texture(depthCubeMap[i], -L).r;
        float lightRange = depthComponent * far_plane;
        float shadow = (r <= lightRange + 0.0005) ? 0.0 : 1.0;

        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = nominator / denominator;

        vec3 k_s = F;
        vec3 k_d = vec3(1.0) - k_s;
        k_d *= 1.0 - metallic;
        float NdotL = max(dot(N, L), 0.0);

        if (mode % 3 == 0)
        {
            Lo += (k_d * albedo / PI + specular) * radiance * NdotL;
        }
        else if (mode % 3 == 1)
        {
            Lo += (k_d * albedo / PI + specular) * radiance * NdotL * (1 - shadow);
        }
        else if (mode % 3 == 2)
        {
            // Lo += r >= lightRange ? 0.0 : 1.0;
            Lo += vec3(depthComponent);
            // Lo += vec3(lightRange / r);
        }
        // Lo += (k_d * albedo / PI + specular) * radiance * NdotL * shadow;
        // Lo += visualizeShadow(pos, i);
    }

    vec3 ambient = vec3(0.03) * albedo * ao * ((mode % 3 == 2) ? 0.0 : 1.0);
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, alpha);
}

