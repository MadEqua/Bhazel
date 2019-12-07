#type vertex
#version 430 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec2 texCoord;
    
out vec3 vCol;
out vec2 vTexCoord;

layout (std140, binding = 0) uniform Frame {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    vec2 timeAndDelta;
};

layout (std140, binding = 1) uniform Instance {
    mat4 modelMatrix;
};

void main() {
    vCol = col;
    vTexCoord = texCoord;

    vec3 mutablePos = pos;
    mutablePos.x += sin(timeAndDelta.x + pos.x * 0.1) * 0.2;
    mutablePos.y += cos(timeAndDelta.x + pos.x * 0.23) * 0.3;

    gl_Position = viewProjectionMatrix * modelMatrix * vec4(mutablePos, 1.0);
}


#type fragment
#version 430 core

layout(location = 0) out vec4 col;

layout(location = 0) uniform sampler2D colorTexture;

in vec3 vCol;
in vec2 vTexCoord;
    
void main() {
    col = texture(colorTexture, vTexCoord);
    //col = vec4(vCol, 1.0);
}