#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPositionAndDirLightCount;
    vec4 dirLightsDirectionsAndIntensities[2];
    vec4 dirLightColors[2];
    float radianceMapMips;
} uSceneConstants;

layout(location = 0) out vec3 outCubeMapDirection;

void main() {
    outCubeMapDirection = attrPosition;
    outCubeMapDirection.x = -outCubeMapDirection.x;

    //Ignore camera position, meaning the box will always be surrounding the camera.
    mat3 rotView = mat3(uSceneConstants.viewMatrix);
    vec3 pos = rotView * attrPosition;

    gl_Position = uSceneConstants.projectionMatrix * vec4(pos, 1.0);

    //Place all vertices on the far clip plane.
    gl_Position.z = gl_Position.w - 0.00001;
}