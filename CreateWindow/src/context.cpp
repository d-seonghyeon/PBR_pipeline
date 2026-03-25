#include "context.h"

ContextUPtr Context::Create(ID3D11Device* device, ID3D11DeviceContext* CTX) {
    auto context = ContextUPtr(new Context());
    if (!context->Init(device, CTX)) return nullptr;
    return std::move(context);
}

bool Context::Init(ID3D11Device* device, ID3D11DeviceContext* context) {
    // 1. 셰이더 로드
    //auto vs = Shader::CreateFromFile("shader/simple.hlsl", "VS", "vs_5_0");
    //auto ps = Shader::CreateFromFile("shader/simple.hlsl", "PS", "ps_5_0");
    auto texVs = Shader::CreateFromFile("shader/texture.hlsl", "VS", "vs_5_0");
    auto texPs = Shader::CreateFromFile("shader/texture.hlsl", "PS", "ps_5_0");

    //if (!vs || !ps || !texVs || !texPs) return false;

    //ShaderPtr vsPtr = std::move(vs);
    //ShaderPtr psPtr = std::move(ps);
    ShaderPtr texVsPtr = std::move(texVs);
    ShaderPtr texPsPtr = std::move(texPs);

    // 2. 이미지 및 텍스처 생성
    auto image = Image::Load("image/brick.jpg");
    if (!image) return false;

    m_texture = Texture::CreateFromImage(device, context, image.get()); 
    if (!m_texture) return false;

    // 3. 레이아웃 생성 (std::move 하기 전에 수행!)
    std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    m_vertexLayout = VertexLayout::Create(device, layoutDesc, texVsPtr->GetBuffer());

    // 4. 프로그램 생성 (여기서 소유권 이전)
    m_textureProgram = Program::Create(device, std::move(texVsPtr), std::move(texPsPtr));
    //m_program = Program::Create(device, std::move(vsPtr), std::move(psPtr));

    //if (!m_textureProgram || !m_program) return false;

    // 5. 정점/인덱스 버퍼 생성 (사용자님 코드와 동일)
    struct Vertex { float pos[3]; float col[4]; float uv[2]; };
    Vertex vertices[] = {
        {{ -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }},
        {{ -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }},
        {{  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }},
        {{  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }}
    };
    uint32_t indices[] = {   
            0, 1, 2,
            0, 2, 3 
        
        };

    m_vertexBuffer = Buffer::CreateWithData(device, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, vertices, sizeof(Vertex), 4);
    m_indexBuffer = Buffer::CreateWithData(device, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DEFAULT, indices, sizeof(uint32_t),6);

    return true;
}
void Context::Render(ID3D11DeviceContext* context) {
    if (!m_texture) return;

    m_texture->Bind(context, 0);

    //m_program->Use(context);
    m_textureProgram->Use(context); 
    m_vertexBuffer->Bind(context, 0);
    m_indexBuffer->Bind(context,0);
    m_vertexLayout->Bind(context);

    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(6,0,0);

}