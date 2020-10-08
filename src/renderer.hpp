#include "global.hpp"
#include "ShaderProgram.hpp"
#include "scene.hpp"
#include "camera.hpp"

class Renderer
{
private:
    Program * pbrProgram;
    Program * shadowProgram;
    GLFWwindow * window;
    Camera * cam;
public:
    Renderer(JSON & config);
    ~Renderer(){};
    void render(Scene & scene);
};

