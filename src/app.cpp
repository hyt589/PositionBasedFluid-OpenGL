#include "app.hpp"


Application::Application(JSON config) : renderer(Renderer(config)) 
{
    auto model = std::make_unique<Model>(config["assets"]["scene"]);
    model->scale = 0.01f;
    scene.addModel(glm::vec3(0.f), glm::vec3(0.f), std::move(model));
    scene.addLight(glm::vec3(0.0f, 8.0f, 0.0f), glm::vec3(1.f), 15000.f);
}

void Application::run(){
    renderer.render(scene);
}