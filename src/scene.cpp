#include "scene.hpp"

void Scene::addLight(glm::vec3 pos, glm::vec3 color, float em){
    if(numLights >= MAX_LIGHTS){
        LOG_ERR("Exceeding max number of lights!")
        return;
    }
    Light l;
    l.position = pos;
    l.color = color;
    l.emission = em;
    lights[numLights] = l;
    numLights++;
}

void Scene::addModel(glm::vec3 pos, glm::vec3 dir, std::unique_ptr<Model> model){
    model->position = pos;
    model->orientation = dir;
    models.push_back(std::move(model));
}

void Scene::render(Program & p){
    for(int i = 0; i < numLights; i++){
        p.setUniform("lightPos[" + std::to_string(i) + "]", lights[i].position, glUniform3fv);
        p.setUniform("lightColor[" + std::to_string(i) + "]", lights[i].color * lights[i].emission, glUniform3fv);
    }
    for(auto & model : models)
    {
        model->Draw(p);
    }
}