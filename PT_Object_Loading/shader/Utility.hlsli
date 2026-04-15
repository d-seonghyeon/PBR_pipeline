
#ifndef __UTILITY_HLSLI__
#define __UTILITY_HLSLI__

// -------------------------------------------------------
// 고급 난수 생성 (Golden Ratio + PCG Hash)
// -------------------------------------------------------

// 32비트 PCG Hash (기존 유지 - 시드 섞기용)
uint PCGHash(uint seed) {
    uint state = seed * 747796405u + 2891336453u;
    uint word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

// 황금비 상수 (Low Discrepancy 수열의 핵심)
static const float GOLDEN_RATIO = 1.61803398875f;

// [0, 1) 범위 float 난수
float UintToFloat01(uint h) {
    return (float)(h & 0x00FFFFFFu) / (float)0x01000000u;
}


 // @brief 개선된 난수 생성기
 // 단순히 무작위인 PCG에 '황금비'를 이용한 시퀀스를 더해 
 // 샘플이 공간상에 더 고르게 분포하도록 만듭니다. (수렴 속도 향상)
 
float GetRandomFloat(uint2 pixelCoord, uint bounce, uint frameCount) {
    // 1. 픽셀 고유의 해시 생성
    uint baseSeed = pixelCoord.x * 1973u + pixelCoord.y * 9277u + bounce * 26699u;
    float h = UintToFloat01(PCGHash(baseSeed));
    
    // 2. 황금비를 더해 매 프레임 샘플이 겹치지 않고 빈 공간을 채우도록 함 (LDS 원리)
    // fract(h + frameCount * GOLDEN_RATIO)
    return frac(h + (float)frameCount * GOLDEN_RATIO);
}


 // @brief 중요도 샘플링용 독립 난수 (2D)
 // R2 시퀀스(수학적 저분산 수열)의 원리를 응용하여 
 // 2차원 평면상에서 샘플이 뭉치지 않게 뿌려줍니다.
 
float2 GetRandomSamples(uint2 pixelCoord, uint bounce, uint frameCount) {
    uint baseSeed = pixelCoord.x * 1973u + pixelCoord.y * 9277u + bounce * 26699u;
    float2 h2 = float2(
        UintToFloat01(PCGHash(baseSeed)),
        UintToFloat01(PCGHash(baseSeed + 31337u))
    );
    
    // 2D상에서 가장 효율적으로 공간을 채우는 상수들 (R2 Sequence)
    const float2 alpha = float2(0.75487766f, 0.56984029f);
    return frac(h2 + (float)frameCount * alpha);
}

// -------------------------------------------------------
// 스카이 컬러 (그라디언트 HDR 하늘) - 기존 유지
// -------------------------------------------------------
float3 GetSkyColor(float3 direction) {
    float t = clamp(direction.y * 0.5f + 0.5f, 0.0f, 1.0f);

    float3 horizon = float3(1.0f, 1.0f, 1.0f);
    float3 zenith  = float3(0.4f, 0.6f, 1.0f);
    float3 sky     = lerp(horizon, zenith, t) * 3.0f;

    float3 sunDir = normalize(float3(0.4f, 0.8f, 0.3f));
    float  sunDot = dot(normalize(direction), sunDir);
    if (sunDot > 0.9995f) {
        sky += float3(20.0f, 18.0f, 14.0f);
    } else if (sunDot > 0.998f) {
        float glow = (sunDot - 0.998f) / (0.9995f - 0.998f);
        sky += lerp(float3(0,0,0), float3(6.0f, 5.0f, 3.0f), glow);
    }

    return sky;
}

#endif
