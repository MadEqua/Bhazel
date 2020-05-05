#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;

layout (set = 2, binding = 0, std140) uniform PassConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
} uPassConstants;

layout(location = 0) out vec3 outCubeMapDirection;


void main() {
    outCubeMapDirection = attrPosition;
    outCubeMapDirection.x = -outCubeMapDirection.x;

    //Ignore camera position, meaning the box will always be surrounding the camera.
    mat3 rotView = mat3(uPassConstants.viewMatrix);
    vec3 pos = rotView * attrPosition;

    gl_Position = uPassConstants.projectionMatrix * vec4(pos, 1.0);

    //Place all vertices on the far clip plane.
    gl_Position.z = gl_Position.w - 0.00001;
}