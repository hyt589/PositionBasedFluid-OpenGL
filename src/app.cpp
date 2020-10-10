#include "app.hpp"


Application::Application(JSON config) : renderer(Renderer(config)) 
{
    auto model = std::make_unique<Model>(config["assets"]["scene"]);
    model->scale = 0.01f;
    scene.addModel(glm::vec3(0.f), glm::vec3(0.f), std::move(model));
    scene.addLight(glm::vec3(-1.0f, 18.0f, 1.0f), glm::vec3(1.f), 1500.f);
}

void Application::run(){
    renderer.render(scene);
}