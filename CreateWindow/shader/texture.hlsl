// texture.hlsl
Texture2D t_diffuse : register(t0);
SamplerState s_diffuse : register(s0); //0번슬롯

struct VS_IN {
    float3 pos : POSITION;//모델 데이터 로컬 좌표
    float4 col : COLOR;
    float2 uv : TEXCOORD; // UV 추가
};

struct PS_IN {
    float4 pos : SV_POSITION;//화면상 최종좌표
    float4 col : COLOR;
    float2 uv : TEXCOORD;
};

PS_IN VS(VS_IN input) {
    PS_IN output;
    output.pos = float4(input.pos, 1.0f);
    output.col = input.col;
    output.uv = input.uv;
    return output;
}

float4 PS(PS_IN input) : SV_Target {
    // 텍스처 색상과 정점 색상을 곱함
    return t_diffuse.Sample(s_diffuse, input.uv) * input.col;
}