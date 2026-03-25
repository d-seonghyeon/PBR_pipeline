#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "vertex_layout.h"
#include "image.h"
#include "texture.h"
#include "mesh.h"

CLASS_PTR(Context)
class Context {
public:
    static ContextUPtr Create(ID3D11Device* device, ID3D11DeviceContext *context);
    void Render(ID3D11DeviceContext* context);

private:
    Context() {}
    bool Init(ID3D11Device* device,ID3D11DeviceContext* context);

    ProgramUPtr m_program;
    ProgramUPtr m_textureProgram;

    VertexLayoutUPtr m_vertexLayout;

    MeshUPtr m_box;


    BufferUPtr m_vertexBuffer;
    BufferUPtr m_indexBuffer;

    ComPtr<ID3D11SamplerState> m_samplerState;
    TextureUPtr m_texture;

    

};

#endif // __CONTEXT_H__