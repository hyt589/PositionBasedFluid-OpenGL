#include "global.hpp"

class Camera
{
private:

    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 up;

public:
    
    Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 up){
        this->pos = pos;
        this->dir = dir;
        this->up = up;
    };

    
    glm::mat4 getViewMatrix();
    ~Camera(){};
};


