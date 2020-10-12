#include "app.hpp"
#include "renderer.hpp"
#include "scene.hpp"

int main(int argc, char const *argv[])
{
    std::string path;
    if (argc < 2)
    {
        path = "config.json";
    } else {
        path = argv[1];
    }

    JSON config;
    std::ifstream in(path);
    in >> config;
    
    // Application app(config);
    // app.run();

    R::OpenGLApplication app(config);
    app.init();
    app.run();

    
    
    return 0;
}
