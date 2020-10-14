#include "scene.hpp"

void Scene::addLight(glm::vec3 pos, glm::vec3 color, float em){
    if(numLights >= MAX_LIGHTS){
        LOG_ERR("Exceeding max number of lights!")
        return;
    }
    Light l;
    l.position = pos;
    l.color = glm::clamp(color, glm::vec3(0.f), glm::vec3(1.f));
    l.emission = em;
    lights[numLights] = l;
    numLights++;
}

void Scene::addLight(Light & l)
{
    if(numLights >= MAX_LIGHTS){
        LOG_ERR("Exceeding max number of lights!")
        return;
    }
    lights[numLights] = l;
    numLights++;
}


void Scene::addModel(glm::vec3 pos, glm::vec3 dir, std::unique_ptr<Model> model){
    model->position = pos;
    model->orientation = dir;
    models.push_back(std::move(model));
}

void Scene::render(Program & p, bool showTexture){
    
    for(auto & model : models)
    {
        model->Draw(p, showTexture);
    }
}