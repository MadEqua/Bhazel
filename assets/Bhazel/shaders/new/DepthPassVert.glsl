#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;


void main() {
    gl_Position = vec4(attrPosition, 1.0);
}