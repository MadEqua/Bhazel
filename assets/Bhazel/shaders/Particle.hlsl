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
    float4 pos : POSITION;
    float4 col : COLOR;
};

struct VsOut {
    float4 col : COLOR;
    float4 pos : SV_POSITION;
};

VsOut main(VsIn input) {
    VsOut res;
    res.col = input.col;
    res.pos = mul(mul(viewProjectionMatrix, modelMatrix), float4(input.pos.xy, 0.0, 1.0));
    return res;
}


#type pixel

struct PsIn {
    float4 col : COLOR;
};

float4 main(PsIn input) : SV_TARGET {
    return input.col;
}