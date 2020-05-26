#version 450 core
#pragma shader_stage(fragment)

layout(location = 0) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform sampler2D uInputTexSampler;

layout(location = 0) out vec4 outColor;

// The minimum amount of local contrast required to apply algorithm.
//   0.333 - too little (faster)
//   0.250 - low quality
//   0.166 - default
//   0.125 - high quality 
//   0.063 - overkill (slower)
const float QUALITY_EDGE_THRESHOLD = 0.125;

// Trims the algorithm from processing darks.
//   0.0833 - upper limit (default, the start of visible unfiltered edges)
//   0.0625 - high quality (faster)
//   0.0312 - visible limit (slower)
const float QUALITY_EDGE_THRESHOLD_MIN = 0.0625;

// Choose the amount of sub-pixel aliasing removal.
// This can effect sharpness.
//   1.00 - upper limit (softer)
//   0.75 - default amount of filtering
//   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
//   0.25 - almost off
//   0.00 - completely off
const float QUALITY_SUBPIX = 0.75;

const int DITHER_STEPS = 8;
const float DITHER_WEIGHTS[DITHER_STEPS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 12.0 };


void main() {
    vec2 qualityRcpFrame = 1.0 / textureSize(uInputTexSampler, 0);
    vec2 posM = inTexCoord;

    vec4 rgbyM = texture(uInputTexSampler, posM);
    #define lumaM rgbyM.w
    vec4 luma4A = textureGather(uInputTexSampler, posM, 3);
    vec4 luma4B = textureGatherOffset(uInputTexSampler, posM, ivec2(-1, -1), 3);
    #define lumaS luma4A.x
    #define lumaSE luma4A.y
    #define lumaE luma4A.z
    #define lumaW luma4B.x
    #define lumaN luma4B.z
    #define lumaNW luma4B.w
    float lumaNE = textureOffset(uInputTexSampler, posM, ivec2(1, -1)).a;
    float lumaSW = textureOffset(uInputTexSampler, posM, ivec2(-1, 1)).a;

    float maxSM = max(lumaS, lumaM);
    float minSM = min(lumaS, lumaM);
    float maxESM = max(lumaE, maxSM);
    float minESM = min(lumaE, minSM);
    float maxWN = max(lumaN, lumaW);
    float minWN = min(lumaN, lumaW);
    float rangeMax = max(maxWN, maxESM);
    float rangeMin = min(minWN, minESM);
    float rangeMaxScaled = rangeMax * QUALITY_EDGE_THRESHOLD;
    float range = rangeMax - rangeMin;
    float rangeMaxClamped = max(QUALITY_EDGE_THRESHOLD_MIN, rangeMaxScaled);
    bool earlyExit = range < rangeMaxClamped;
    if(earlyExit) {
        outColor = vec4(rgbyM.rgb, 1.0);
        return;
    }

    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;
    float subpixRcpRange = 1.0 / range;
    float subpixNSWE = lumaNS + lumaWE;
    float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
    float edgeVert1 = (-2.0 * lumaM) + lumaWE;

    float lumaNESE = lumaNE + lumaSE;
    float lumaNWNE = lumaNW + lumaNE;
    float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
    float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;

    float lumaNWSW = lumaNW + lumaSW;
    float lumaSWSE = lumaSW + lumaSE;
    float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
    float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
    float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
    float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
    float edgeHorz = abs(edgeHorz3) + edgeHorz4;
    float edgeVert = abs(edgeVert3) + edgeVert4;

    float subpixNWSWNESE = lumaNWSW + lumaNESE;
    float lengthSign = qualityRcpFrame.x;
    bool horzSpan = edgeHorz >= edgeVert;
    float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;

    if(!horzSpan) lumaN = lumaW;
    if(!horzSpan) lumaS = lumaE;
    if(horzSpan) lengthSign = qualityRcpFrame.y;
    float subpixB = (subpixA * (1.0/12.0)) - lumaM;

    float gradientN = lumaN - lumaM;
    float gradientS = lumaS - lumaM;
    float lumaNN = lumaN + lumaM;
    float lumaSS = lumaS + lumaM;
    bool pairN = abs(gradientN) >= abs(gradientS);
    float gradient = max(abs(gradientN), abs(gradientS));
    if(pairN) lengthSign = -lengthSign;
    float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);

    vec2 posB;
    posB.x = posM.x;
    posB.y = posM.y;
    vec2 offNP;
    offNP.x = (!horzSpan) ? 0.0 : qualityRcpFrame.x;
    offNP.y = (horzSpan) ? 0.0 : qualityRcpFrame.y;
    if(!horzSpan) posB.x += lengthSign * 0.5;
    if(horzSpan) posB.y += lengthSign * 0.5;

    vec2 posN;
    posN.x = posB.x - offNP.x * DITHER_WEIGHTS[0];
    posN.y = posB.y - offNP.y * DITHER_WEIGHTS[0];
    vec2 posP;
    posP.x = posB.x + offNP.x * DITHER_WEIGHTS[0];
    posP.y = posB.y + offNP.y * DITHER_WEIGHTS[0];
    float subpixD = ((-2.0)*subpixC) + 3.0;
    float lumaEndN = texture(uInputTexSampler, posN).a;
    float subpixE = subpixC * subpixC;
    float lumaEndP = texture(uInputTexSampler, posP).a;

    if(!pairN) lumaNN = lumaSS;
    float gradientScaled = gradient * 1.0 / 4.0;
    float lumaMM = lumaM - lumaNN * 0.5;
    float subpixF = subpixD * subpixE;
    bool lumaMLTZero = lumaMM < 0.0;

    lumaEndN -= lumaNN * 0.5;
    lumaEndP -= lumaNN * 0.5;
    bool doneN = abs(lumaEndN) >= gradientScaled;
    bool doneP = abs(lumaEndP) >= gradientScaled;
    if(!doneN) posN.x -= offNP.x * DITHER_WEIGHTS[1];
    if(!doneN) posN.y -= offNP.y * DITHER_WEIGHTS[1];
    bool doneNP = (!doneN) || (!doneP);
    if(!doneP) posP.x += offNP.x * DITHER_WEIGHTS[1];
    if(!doneP) posP.y += offNP.y * DITHER_WEIGHTS[1];

    for(int i = 2; i < DITHER_STEPS; ++i) {
        if(doneNP) {
            if(!doneN) lumaEndN = texture(uInputTexSampler, posN.xy).a;
            if(!doneP) lumaEndP = texture(uInputTexSampler, posP.xy).a;
            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;

            doneN = abs(lumaEndN) >= gradientScaled;
            doneP = abs(lumaEndP) >= gradientScaled;
            if(!doneN) posN.x -= offNP.x * DITHER_WEIGHTS[i];
            if(!doneN) posN.y -= offNP.y * DITHER_WEIGHTS[i];
            doneNP = (!doneN) || (!doneP);
            if(!doneP) posP.x += offNP.x * DITHER_WEIGHTS[i];
            if(!doneP) posP.y += offNP.y * DITHER_WEIGHTS[i];
        }
        else break;
    }

    float dstN = posM.x - posN.x;
    float dstP = posP.x - posM.x;
    if(!horzSpan) dstN = posM.y - posN.y;
    if(!horzSpan) dstP = posP.y - posM.y;

    bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
    float spanLength = (dstP + dstN);
    bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
    float spanLengthRcp = 1.0/spanLength;

    bool directionN = dstN < dstP;
    float dst = min(dstN, dstP);
    bool goodSpan = directionN ? goodSpanN : goodSpanP;
    float subpixG = subpixF * subpixF;
    float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
    float subpixH = subpixG * QUALITY_SUBPIX;

    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
    float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);

    if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
    if(horzSpan) posM.y += pixelOffsetSubpix * lengthSign;

    outColor = vec4(texture(uInputTexSampler, posM).rgb, 1.0);
}