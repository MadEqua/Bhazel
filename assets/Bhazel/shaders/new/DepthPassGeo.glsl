#version 450 core
#pragma shader_stage(geometry)

#define SHADOW_MAPPING_CASCADE_COUNT 4

layout (invocations = SHADOW_MAPPING_CASCADE_COUNT, triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (set = 2, binding = 0, std140) uniform PassConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
} uPassConstants[SHADOW_MAPPING_CASCADE_COUNT];

layout (set = 4, binding = 0, std140) uniform EntityConstants {
    mat4 modelMatrix;
    mat4 normalMatrix;
} uEntityConstants;


void main() {
    for(int i = 0; i < gl_in.length(); ++i) {
        gl_Position = uPassConstants[gl_InvocationID].viewProjectionMatrix * uEntityConstants.modelMatrix * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}