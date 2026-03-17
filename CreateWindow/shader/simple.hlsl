// 1. CPU에서 GPU로 넘어오는 정점 데이터 구조 (VertexLayout과 일치해야 함)
struct VS_INPUT {
    float3 pos : POSITION;   // 위치 (x, y, z)
    float4 color : COLOR;    // 색상 (r, g, b, a)
};

// 2. Vertex Shader에서 Pixel Shader로 넘어가는 데이터 구조
struct PS_INPUT {
    float4 pos : SV_POSITION; // 시스템 예약용 위치 (Screen Space)
    float4 color : COLOR;      // 보간된 색상
};

// --- Vertex Shader ---
// 정점의 위치를 변환하고 픽셀 셰이더로 데이터를 넘깁니다.
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output;
    
    // 현재는 단순 2D 삼각형이므로 변환 없이 4차원 벡터로만 확장합니다.
    output.pos = float4(input.pos, 1.0f); 
    output.color = input.color;
    
    return output;
}

// --- Pixel Shader ---
// 각 픽셀의 최종 색상을 결정합니다.
float4 PS(PS_INPUT input) : SV_Target {
    // Vertex Shader에서 넘어온 색상을 그대로 출력합니다.
    // 점 사이의 영역은 GPU가 자동으로 색상을 섞어(보간) 줍니다.
    return input.color;
}