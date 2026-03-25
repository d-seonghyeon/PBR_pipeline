#ifndef __MESH_H__
#define __MESH_H__

#include "common.h"
#include "buffer.h"
#include "vertex_layout.h"
#include "texture.h"
#include "program.h"

// 정점 구조체: HLSL의 인풋 레이아웃과 일치해야 합니다.
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

// Material: 셰이더에 텍스처와 속성을 전달
CLASS_PTR(Material);
class Material {
public:
    static MaterialUPtr Create() {
        return MaterialUPtr(new Material());
    }
    TexturePtr diffuse;
    TexturePtr specular;
    float shininess {32.0f};

    // DX11에서는 Context가 필요합니다.
    void SetToProgram(ID3D11DeviceContext* context) const;

private:
    Material() {}
};

// Mesh: 정점/인덱스 버퍼 관리 및 그리기
CLASS_PTR(Mesh);
class Mesh {
public:
    static MeshUPtr Create(
        ID3D11Device* device,
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices,
        D3D11_PRIMITIVE_TOPOLOGY primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    );

    static MeshUPtr CreateBox(ID3D11Device* device);

    const VertexLayout* GetVertexLayout() const { return m_vertexLayout.get(); }
    BufferPtr GetVertexBuffer() const { return m_vertexBuffer; }
    BufferPtr GetIndexBuffer() const { return m_indexBuffer; }

    void SetMaterial(MaterialPtr material) { m_material = material; }
    MaterialPtr GetMaterial() const { return m_material; }

    // 그리기 함수: DX11 Context를 인자로 받습니다.
    void Draw(ID3D11DeviceContext* context, const Program* program) const;

private:
    Mesh() {}

    void Init(
        ID3D11Device* device,
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices,
        D3D11_PRIMITIVE_TOPOLOGY primitiveType
    );

    MaterialPtr m_material;

    // OpenGL의 GL_TRIANGLES 대신 DX11의 Topology를 사용합니다.
    D3D11_PRIMITIVE_TOPOLOGY m_primitiveType{ D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
    VertexLayoutUPtr m_vertexLayout;
    BufferPtr m_vertexBuffer;
    BufferPtr m_indexBuffer;
};

#endif