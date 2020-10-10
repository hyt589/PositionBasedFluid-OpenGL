#pragma once

#include "global.hpp"

class Camera
{

public:

    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 up;

    float speed = 1.f, sensitivity = 0.1f;
    float yaw = 0.f, pitch = 0.f;
    
    Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 up){
        this->pos = pos;
        this->dir = dir;
        this->up = up;

        // pitch = asin(dir.z);
        // yaw = atan2(dir.y, dir.x);
    };

    
    glm::mat4 getViewMatrix();
    ~Camera(){};
};


