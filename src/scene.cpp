#include "scene.hpp"

Scene::Scene()
{
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        GL(glGenFramebuffers(1, &(depthMapFBO[i])));  

        GL(glGenTextures(1, &(depthMap[i])));
        GL(glBindTexture(GL_TEXTURE_2D, depthMap[i]));
        GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

        GL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]));
        GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap[i], 0));
        GL(glDrawBuffer(GL_NONE));
        GL(glReadBuffer(GL_NONE));
        GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    
}

void Scene::addLight(glm::vec3 pos, glm::vec3 color, float em){
    if(numLights >= MAX_LIGHTS){
        LOG_WARN("At file: " << __FILE__ << ", line: " << __LINE__)
        LOG_WARN("Exceeding max light number: " << MAX_LIGHTS)
        return;
    }
    Light l;
    l.position = pos;
    l.color = glm::max(color, glm::vec3(1.f));
    l.emission = em;
    lights[numLights] = l;
    numLights ++;
}

void Scene::addModel(glm::vec3 pos, glm::vec3 dir, std::unique_ptr<Model> model){
    model->position = pos;
    model->orientation = dir;
    models.push_back(std::move(model));
}

void Scene::render(Program &scene, Program &shadow, GLFWwindow * window){

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    for (int i = 0; i < numLights; i++)
    {
        GL(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
        GL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]));
        GL(glClear(GL_DEPTH_BUFFER_BIT));
        shadow.setUniform("mat_view", glm::lookAt(lights[i].position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f)), glUniform3fv);
        shadow.setUniform("mat_projection", projection, glUniformMatrix4fv);
        for(auto & model : models)
        {
            model->Draw(shadow);
        }
        GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    
    GL(glViewport(0, 0, windowWidth, windowHeight));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    scene.setUniform("mat_view", cam->getViewMatrix(), glUniformMatrix4fv);
    scene.setUniform("mat_projection", projection, glUniformMatrix4fv);
    for(int i = 0; i < numLights; i++){
        scene.setUniform("lightPos[" + std::to_string(i) + "]", lights[i].position, glUniform3fv);
        scene.setUniform("lightColor[" + std::to_string(i) + "]", lights[i].color, glUniform3fv);
        scene.setUniform("lightEmission[" + std::to_string(i) + "]", lights[i].emission, glUniform1f);
    }
    for(auto & model : models)
    {
        model->Draw(scene);
    }
}

Scene::~Scene()
{
}