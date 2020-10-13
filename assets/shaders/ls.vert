#version 410 core

layout (location = 0) in vec3 vPos;


uniform mat4 mat_model;
uniform mat4 mat_view;
uniform mat4 mat_projection;


void main(){
    // uv = v_uv;
    // pos = vec3(mat_model * vec4(vPos, 1.0));
    // normal = mat3(mat_model) * vNormal;

    gl_Position = mat_projection * mat_view * mat_model * vec4(vPos, 1.0);
}
