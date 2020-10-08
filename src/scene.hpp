#pragma once

#include "global.hpp"
#include "asset.hpp"
#include "camera.hpp"
#include "ShaderProgram.hpp"

struct Light
{
    glm::vec3 position;
    glm::vec3 color;
    float emission;
};

class Scene
{
private:
    std::array<Light, MAX_LIGHTS> lights;
    std::vector<std::unique_ptr<Model>> models;
    glm::mat4 projection;
    int numLights = 0;
    uint depthMap[MAX_LIGHTS];
    uint depthMapFBO[MAX_LIGHTS];
    const uint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
public:
    Scene(){};
    void addLight(glm::vec3 pos, glm::vec3 color, float em);
    void addModel(glm::vec3 pos, glm::vec3 dir, std::unique_ptr<Model> model);
    void setProjection(glm::mat4);
    void render(Program & p);
    ~Scene(){};
};


