#type vertex
#version 430 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords;
    
out vec3 vTint;
out vec2 vTexCoords;

layout (std140, binding = 0) uniform Frame {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    vec2 timeAndDelta;
};

struct Particle {
    vec4 positionAndLife;
    vec3 velocity;
    vec3 acceleration;
    vec3 tint;
    vec2 size;
};

layout(std140, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};


void main() {
    Particle p = particles[gl_InstanceID];

    //Billboard
    vec3 worldPos = p.positionAndLife.xyz;

    vec3 z = normalize(cameraPosition - worldPos);
    vec3 x = normalize(cross(vec3(0, 1, 0), z));
    vec3 y = normalize(cross(z, x));
    
    /*mat4 rotMatrix = mat4(vec4(x, 0.0), vec4(y, 0.0), vec4(z, 0.0), vec4(0, 0, 0, 1));
    mat4 scaleMatrix = mat4(p.size.x, 0.0, 0.0, 0.0,
                            0.0, p.size.y, 0.0, 0.0,
                            0.0, 0.0, 1.0, 0.0,
                            0.0, 0.0, 0.0, 1.0);

    mat4 transMatrix = mat4(1.0, 0.0, 0.0, 0.0,
                            0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 1.0, 0.0,
                            worldPos.x, worldPos.y, worldPos.z, 1.0);*/

    mat4 modelMatrix = mat4(p.size.x * x.x, p.size.x * x.y, p.size.x * x.z, 0.0f,
                            p.size.y * y.x, p.size.y * y.y, p.size.y * y.z, 0.0f,
                            z.x,            z.y,            z.z, 0.0f,
                            worldPos.x, worldPos.y, worldPos.z, 1.0f);

    gl_Position = viewProjectionMatrix * modelMatrix * vec4(position, 0.0, 1.0);

    vTexCoords = texCoords;
    vTint = p.tint.rgb;
}


#type fragment
#version 430 core

layout(location = 0) out vec4 col;

in vec3 vTint;
in vec2 vTexCoords;

uniform sampler2D colorTexture;
    
void main() {
    col = vec4(vTint, 1.0) * texture(colorTexture, vTexCoords);
}