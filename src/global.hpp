#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>
#include <json.hpp>
#include <fstream>
#include <set>
#include <map>
#include <memory>
#include <array>
#include <any>
#include <thread>
#include <csignal>
#include <functional>

using JSON = nlohmann::json;

#ifndef NDEBUG
#define LOG_INFO(msg) std::cout << "[Info] " << msg << std::endl;
#define LOG_ERR(msg) std::cerr << "[Error] " << msg << std::endl;
#define LOG_WARN(msg) std::cout << "[Warning] " << msg << std::endl;
#define BREAK_POINT LOG_WARN("Execution Paused from file: '" << __FILE__ << "', line " << __LINE__) std::raise(SIGINT)
#else
#define LOG_INFO(msg)
#define LOG_ERR(msg)
#define LOG_WARN(msg)
#define BREAK_POINT
#endif // !NDEBUG


inline bool glCheckError_(const char * file, int line)
{
    bool err = false;
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "GL_INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "GL_STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "GL_STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        LOG_ERR(error << " occured at file '" << file << "', line " << line);
        err = true;
        // std::raise(SIGINT);
    }
    return err;
}
#define glCheckError if(glCheckError_(__FILE__, __LINE__)) std::raise(SIGINT)

#ifdef NDEBUG
#define GL(glFunctionCall) glFunctionCall
#else
#define GL(glFunctionCall) glFunctionCall; glCheckError
#endif // NDEBUG


template<typename Fn, Fn fn, typename... Args>
typename std::result_of<Fn(Args...)>::type
wrapper(Args&&... args) {
    return fn(std::forward<Args>(args)...);
}

#define COMPUTE(FUNC, ...) wrapper<decltype(&FUNC), &FUNC>(__VA_ARGS__)

#define MAX_LIGHTS 4