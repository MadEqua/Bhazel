#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) out vec2 outTexCoord;


void main() {
    vec2 vertices[3] = vec2[3](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
    gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);
    outTexCoord = 0.5 * gl_Position.xy + vec2(0.5);
}