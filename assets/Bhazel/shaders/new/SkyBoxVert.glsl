#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;

layout (set = 0, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
    vec4 dirLightsDirectionsAndIntensities[2];
    vec4 dirLightColors[2];
    int dirLightsCount;
} uSceneConstants;

layout(location = 0) out vec3 outCubeMapDirection;

void main() {
    outCubeMapDirection = attrPosition;

    //Ignore camera position, meaning the box will always be surronding the camera.
    mat3 rotView = mat3(uSceneConstants.viewMatrix);
    vec3 pos = rotView * attrPosition;

    gl_Position = uSceneConstants.projectionMatrix * vec4(pos, 1.0);

    //Place all vertices on the far clip plane.
    gl_Position.z = gl_Position.w - 0.00001;
}