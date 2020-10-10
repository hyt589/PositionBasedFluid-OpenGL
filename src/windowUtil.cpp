#include "windowUtil.hpp"


void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    GL(glViewport(0, 0, width, height));
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto r = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        r->mode++;
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}

void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
    auto r = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !r->focus)
    {
        r->focus = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // glfwSetCursorPosCallback(window, cursorPosCallback);
    }

    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && r->focus)
    {
        r->focus = false;
        r->mouseInit = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        // glfwSetCursorPosCallback(window, nullptr);
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto r = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if(r->focus){
        if(!r->mouseInit){
            r->lastx = xpos;
            r->lasty = ypos;
            r->mouseInit = true;
        }

        float xoffset = xpos - r->lastx;
        float yoffset = r->lasty - ypos;
        r->lastx = xpos;
        r->lasty = ypos;

        xoffset *= r->cam->sensitivity;
        yoffset *= r->cam->sensitivity;

        r->cam->yaw   += xoffset;
        r->cam->pitch += yoffset;

        if(r->cam->pitch > 89.0f)
            r->cam->pitch = 89.0f;
        if(r->cam->pitch < -89.0f)
            r->cam->pitch = -89.0f;

        glm::vec3 nextDir(
            cos(glm::radians(r->cam->yaw)) * cos(glm::radians(r->cam->pitch)),
            sin(glm::radians(r->cam->pitch)),
            sin(glm::radians(r->cam->yaw)) * cos(glm::radians(r->cam->pitch))
        );

        r->cam->dir = nextDir;
    }
}
