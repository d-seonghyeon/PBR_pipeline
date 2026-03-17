#include "program.h"

ProgramUPtr Program::Create(ID3D11Device* device, ShaderPtr vs, ShaderPtr ps) {
    auto program = ProgramUPtr(new Program());
    if (!program->Init(device, vs, ps)) {
        return nullptr;
    }
    return std::move(program);
}

Program::~Program() {}

bool Program::Init(ID3D11Device* device, ShaderPtr vs, ShaderPtr ps) {
    if (!vs || !ps) return false;

    // Blob 데이터를 이용해 실제 GPU 셰이더 객체 생성
    HRESULT hrVS = device->CreateVertexShader(
        vs->GetBuffer()->GetBufferPointer(), 
        vs->GetBuffer()->GetBufferSize(), 
        nullptr, &m_vs
    );
    if (FAILED(hrVS)) {
        SPDLOG_ERROR("Failed to create Vertex Shader");
        return false;
    }

    HRESULT hrPS = device->CreatePixelShader(
        ps->GetBuffer()->GetBufferPointer(), 
        ps->GetBuffer()->GetBufferSize(), 
        nullptr, &m_ps
    );
    if (FAILED(hrPS)) {
        SPDLOG_ERROR("Failed to create Pixel Shader");
        return false;
    }

    return true;
}

void Program::Use(ID3D11DeviceContext* context) const {
    context->VSSetShader(m_vs.Get(), nullptr, 0);
    context->PSSetShader(m_ps.Get(), nullptr, 0);
}