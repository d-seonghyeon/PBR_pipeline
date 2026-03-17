#include "context.h"

ContextUPtr Context::Create(ID3D11Device* device) {
    auto context = ContextUPtr(new Context());
    if (!context->Init(device)) return nullptr;
    return std::move(context);
}

bool Context::Init(ID3D11Device* device) {

    std::string shaderPath = "shader/simple.hlsl";
    
    auto vs = Shader::CreateFromFile(shaderPath, "VS", "vs_5_0");
    if (!vs) { SPDLOG_ERROR("VS Load Failed!"); return false; }
    
    auto ps = Shader::CreateFromFile(shaderPath, "PS", "ps_5_0");
    if (!ps) { SPDLOG_ERROR("PS Load Failed!"); return false; }

    ShaderPtr vsPtr = std::move(vs);
    ShaderPtr psPtr = std::move(ps);

    m_program = Program::Create(device, vsPtr, psPtr);
    if (!m_program) return false;

    struct Vertex { float pos[3]; float col[4]; };
    Vertex vertices[] = {
        {{ 0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
    };

    m_vertexBuffer = Buffer::CreateWithData(
        device, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, 
        vertices, sizeof(vertices), sizeof(Vertex)
    );

    std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    m_vertexLayout = VertexLayout::Create(device, layoutDesc, vsPtr->GetBuffer());

    return true;
}

void Context::Render(ID3D11DeviceContext* context) {
    m_program->Use(context);
    m_vertexBuffer->Bind(context, 0);
    m_vertexLayout->Bind(context);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->Draw(3, 0);
}