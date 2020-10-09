#version 410 core

layout (location = 0) in vec3 vPos;

uniform mat4 mat_model;

void main(){
    gl_Position = mat_model * vec4(vPos, 1.0);
}