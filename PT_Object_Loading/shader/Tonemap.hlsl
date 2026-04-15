// HDR 누적 버퍼 → LDR 백버퍼 변환
Texture2D<float4>           g_hdrInput  : register(t10);
RWTexture2D<unorm float4>   g_ldrOutput : register(u1);

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID) {
    uint w, h;
    g_ldrOutput.GetDimensions(w, h);
    if (id.x >= w || id.y >= h) return;

    float3 hdr = g_hdrInput[id.xy].rgb;

    // Reinhard 톤맵핑
    float3 ldr = hdr / (hdr + 1.0f);

    // 감마 보정
    ldr = pow(saturate(ldr), 1.0f / 2.2f);

    // 또는 정확한 sRGB:
    // if (color <= 0.0031308) color = 12.92 * color;
    // else color = 1.055 * pow(color, 1.0/2.4) - 0.055;

    g_ldrOutput[id.xy] = float4(ldr, 1.0f);
}


/*톤맵핑 ->감마 보정 순서 주의

0~1로 밝기 조절을 한 후 감마 보정을 해야 0~1의 밝기가 유지가 됨



*/

//----texture 읽을때------
/*
텍스처 읽기 (sRGB → linear)
albedo = pow(texture_color, vec3(2.2));

모든 셰이딩 계산 (선형 공간에서)
result = brdf * light * cos_theta;

최종 출력 (linear → sRGB)
output = pow(tonemap(result), vec3(1.0/2.2));

만약 텍스처를 읽을때 2.2배를 안하고 읽어오면 감마보정이 2번이 들어가 밝기 지나치게 밝고, 대비가 낮아짐
*/