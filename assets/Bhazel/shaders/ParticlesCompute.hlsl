#type compute

cbuffer Frame : register(b0) {
    float4x4 viewMatrix;
    float4x4 ProjectionMatrix;
    float4x4 viewProjectionMatrix;
    float time;
};

/*struct VxData {
    float4 pos;
    float4 col;
};*/

//StructuredBuffer<VxData> ParticleBuffer : register(t0);
RWByteAddressBuffer ParticleBuffer ;


float2 randomVec2(float v) {
    return frac(float2(sin(v * 5474.241), sin(v * 1265.34)) * 4892.43);
}

float randomF(float v) {
    return frac(sin(v * 5642.34) * 5437.43);
}

[numthreads(128, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint idx = dispatchThreadID.x * 8 * 4;

    float2 posEdit = float2(asfloat(ParticleBuffer.Load(idx)), asfloat(ParticleBuffer.Load(idx + 4)));

    float2 axis = randomVec2(dispatchThreadID.x) * 2.0 - 1.0;
    posEdit.xy += normalize(axis) * randomF(dispatchThreadID.x) * 0.05 * sign(sin(time * 2.));

    ParticleBuffer.Store(idx, asuint(posEdit.x));
    ParticleBuffer.Store(idx + 4, asuint(posEdit.y));
} 