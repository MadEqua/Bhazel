#type vertex

cbuffer Frame : register(b0) {
    float4x4 viewMatrix;
    float4x4 ProjectionMatrix;
    float4x4 viewProjectionMatrix;
    float time;
};

cbuffer Instance : register(b1) {
    float4x4 modelMatrix;
};

struct VsIn {
    float3 pos : POSITION;
    float3 col : COLOR;
    float2 texCoord : TEXCOORD;
};

struct VsOut {
    float3 col : COLOR;
    float2 texCoord : TEXCOORD;
    float4 pos : SV_POSITION;
};

VsOut main(VsIn input) {
    VsOut res;
    res.col = input.col;
    res.texCoord = input.texCoord;

    input.pos.x += sin(time + input.pos.x * 0.1) * 0.2;
    input.pos.y += cos(time + input.pos.x * 0.23) * 0.3;

    res.pos = mul(mul(viewProjectionMatrix, modelMatrix), float4(input.pos, 1.0));
    return res;
}


#type pixel

struct PsIn {
    float3 col : COLOR;
    float2 texCoord : TEXCOORD;
};

Texture2D tex;
SamplerState splr;

float4 main(PsIn input) : SV_TARGET {
    //return float4(input.col, 1.0);
    return tex.Sample(splr, input.texCoord);
}