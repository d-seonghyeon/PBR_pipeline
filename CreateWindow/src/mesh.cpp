#include "mesh.h"

MeshUPtr Mesh::Create(
    ID3D11Device* device,
    const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices,
    D3D11_PRIMITIVE_TOPOLOGY primitiveType) 
{
    auto mesh = MeshUPtr(new Mesh());
    mesh->Init(device, vertices, indices, primitiveType);
    return std::move(mesh);
}

void Mesh::Init(
    ID3D11Device* device,
    const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices,
    D3D11_PRIMITIVE_TOPOLOGY primitiveType) 
{
    m_primitiveType = primitiveType;

    // 1. 버퍼 생성 (GL_ARRAY_BUFFER -> D3D11_BIND_VERTEX_BUFFER)
    m_vertexBuffer = Buffer::CreateWithData(
        device, 
        D3D11_BIND_VERTEX_BUFFER, 
        D3D11_USAGE_DEFAULT,
        vertices.data(), 
        sizeof(Vertex) * vertices.size(), 
        sizeof(Vertex)
    );

    m_indexBuffer = Buffer::CreateWithData(
        device, 
        D3D11_BIND_INDEX_BUFFER, 
        D3D11_USAGE_DEFAULT,
        indices.data(), 
        sizeof(uint32_t) * indices.size(), 
        sizeof(uint32_t)
    );

    // 2. 입력 레이아웃 설정 (DirectX 11 스타일)
    // 주의: VertexLayout 생성 시 셰이더 바이트코드가 필요할 수 있습니다. 
    // 여기서는 기존 VertexLayout::Create가 어떻게 구현되었느냐에 따라 맞춰야 합니다.
    std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(Vertex, texCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    
    // VertexLayout 생성 (셰이더 바이너리는 별도로 관리하거나 전역적으로 전달받아야 함)
    // m_vertexLayout = VertexLayout::Create(device, layoutDesc, shaderByteCode);
}

void Mesh::Draw(ID3D11DeviceContext* context, const Program* program) const {
    // 1. 상태 바인딩
    m_vertexBuffer->Bind(context, 0);
    m_indexBuffer->Bind(context, 0);
    if (m_vertexLayout) m_vertexLayout->Bind(context);

    // 2. 머티리얼 설정
    if (m_material) {
        m_material->SetToProgram(context);
    }

    // 3. 토폴로지 설정 및 그리기
    context->IASetPrimitiveTopology(m_primitiveType);
    context->DrawIndexed(m_indexBuffer->GetCount(), 0, 0);
}

MeshUPtr Mesh::CreateBox(ID3D11Device* device) {
        std::vector<Vertex> vertices = {
        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f) },

        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 1.0f) },

        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 0.0f) },
    };

    std::vector<uint32_t> indices = {
        0,  2,  1,  2,  0,  3,
        4,  5,  6,  6,  7,  4,
        8,  9, 10, 10, 11,  8,
        12, 14, 13, 14, 12, 15,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 22, 20, 23,
    };

    return Create(device, vertices, indices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

