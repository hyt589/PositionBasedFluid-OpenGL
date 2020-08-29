#include "camera.hpp"


glm::mat4 Camera::getViewMatrix(){
    auto target = pos + dir;

    return glm::lookAt(pos, target, up);
}