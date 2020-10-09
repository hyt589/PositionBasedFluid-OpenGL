#include "windowUtil.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    GL(glViewport(0, 0, width, height));
}

