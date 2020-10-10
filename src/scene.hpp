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
    
public:
    std::array<Light, MAX_LIGHTS> lights;
    std::vector<std::unique_ptr<Model>> models;
    int numLights = 0;
    Scene(){};
    void addLight(glm::vec3 pos, glm::vec3 color, float em);
    void addModel(glm::vec3 pos, glm::vec3 dir, std::unique_ptr<Model> model);
    void render(Program & p, bool showTexture);
    ~Scene(){};
};


