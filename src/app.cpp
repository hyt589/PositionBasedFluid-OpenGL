#include "app.hpp"


Application::Application(JSON config) : renderer(Renderer(config)) 
{
    auto model = std::make_unique<Model>(config["assets"]["scene"]);
    scene.addModel(glm::vec3(0.f), glm::vec3(0.f), std::move(model));
    scene.addLight(glm::vec3(10.0f, 1200.0f, 10.0f), glm::vec3(1.f), 1500000.f);
}

void Application::run(){
    renderer.render(scene);
}