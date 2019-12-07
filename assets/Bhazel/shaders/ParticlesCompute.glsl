#type compute
#version 430 core

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

struct Particle {
    vec4 positionAndLife;
    vec3 velocity;
    vec3 acceleration;
    vec3 tint;
    vec2 size;
};

struct RangeVec3 {
   vec3 minimum;
   vec3 maximum;
};

struct RangeFloat {
   float minimum;
   float maximum;
};

layout (std140, binding = 2) uniform ParticleRanges {
    RangeVec3 positionRange;
    RangeVec3 sizeRange;
    RangeFloat lifeRange;
    RangeVec3 velocityRange;
    RangeVec3 accelerationRange;
    RangeVec3 tintRange;
};

layout(std140, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

float hash(float n) {
    return fract(sin(n * 6562.432) * 4358.5453);
}

float randRange(float minimum, float maximum, float seed) {
    float dif = abs(maximum - minimum);
    return (hash(0.001 * timeAndDelta.x + float(gl_GlobalInvocationID.x) + seed) * dif) + minimum;
}

float randRange(RangeFloat range, float seed) {
    return randRange(range.minimum, range.maximum, seed);
}

vec3 randRange(RangeVec3 range, vec3 seed) {
    return vec3(randRange(range.minimum.x, range.maximum.x, seed.x),
                randRange(range.minimum.y, range.maximum.y, seed.y),
                randRange(range.minimum.z, range.maximum.z, seed.z));
}

vec3 localToWorldP(vec3 p) {
    return (modelMatrix * vec4(p, 1.0)).xyz;
}

vec3 localToWorldV(vec3 v) {
    return (modelMatrix * vec4(v, 0.0)).xyz;
}

void main() {
    uint idx = gl_GlobalInvocationID.x;
    Particle p = particles[idx];

    if(p.positionAndLife.w <= 0) {
        p.positionAndLife.xyz = localToWorldP(randRange(positionRange, vec3(213.3, 45.4, 764.3)));
        p.positionAndLife.w = randRange(lifeRange, 433.54);
        p.velocity = localToWorldV(randRange(velocityRange, vec3(1213.34, 15.67, 765.54)));
        p.acceleration = localToWorldV(randRange(accelerationRange, vec3(6543.3, 876.4, 4453.7)));
        p.tint = randRange(tintRange, vec3(765.534, 542.7654, 465.786));
        p.size = randRange(sizeRange, vec3(432.34, 876.67, 32.54)).xy;
        particles[idx] = p;
    }
    else {
        vec3 pos = p.positionAndLife.xyz;
        vec3 vel = p.velocity;
        vec3 accel = p.acceleration;
        //vec3 tint = p.tint;

        p.positionAndLife.xyz += p.velocity * timeAndDelta.y;
        p.positionAndLife.w -= timeAndDelta.y;
        p.velocity += p.acceleration * timeAndDelta.y;
        //p.acceleration *= 0.9;

        particles[idx] = p;
    }
}