#version 410 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 v_uv;

out vec2 uv;
out vec3 pos;
out vec3 normal;
out vec4 ligghtSpaceFragPos;

uniform mat4 mat_model;
uniform mat4 mat_view;
uniform mat4 mat_projection;
uniform mat4 mat_lightSpace;


void main(){
    uv = v_uv;
    pos = vec3(mat_model * vec4(vPos, 1.0));
    normal = mat3(mat_model) * vNormal;

    gl_Position = mat_projection * mat_view * mat_model * vec4(vPos, 1.0);
}
