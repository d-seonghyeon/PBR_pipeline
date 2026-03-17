#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "vertex_layout.h"

CLASS_PTR(Context)
class Context {
public:
    static ContextUPtr Create(ID3D11Device* device);
    void Render(ID3D11DeviceContext* context);

private:
    Context() {}
    bool Init(ID3D11Device* device);

    ProgramUPtr m_program;
    VertexLayoutUPtr m_vertexLayout;
    BufferUPtr m_vertexBuffer;
};

#endif // __CONTEXT_H__