#include "context.h"
#include <spdlog/spdlog.h>

ContextUPtr Context::Create(ID3D11Device* device, ID3D11DeviceContext* ctx) {
    auto context = ContextUPtr(new Context());
    if (!context->Init(device, ctx)) return nullptr;
    return std::move(context);
}

bool Context::Init(ID3D11Device* device, ID3D11DeviceContext* context) {
    // 1. 패스 트레이서 컴퓨트 셰이더 로드
    auto cs = Shader::CreateFromFile("shader/PathTracer.hlsl", "CSMain", "cs_5_0");
    if (!cs) return false;
    m_pathTracerProgram = ComputeProgram::Create(device, std::move(cs));
    if (!m_pathTracerProgram) return false;

    // 2. 글로벌 상수 버퍼
    m_globalBuffer = Buffer::CreateWithData(
        device, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC,
        nullptr, sizeof(GlobalUniforms), 1
    );
    if (!m_globalBuffer) return false;

    // 3. 모델 로딩
    // 파일이 없으면 기본 씬(구+평면)으로 폴백
    m_model = Model::Load(device, "model/backpack.obj");
    if (!m_model) {
        SPDLOG_WARN("Model not found, using default scene.");
    }

    // 4. 씬 버퍼 빌드 (모든 메시를 하나의 큰 버퍼로 합치기)
    if (!BuildSceneBuffers(device)) return false;

    // 5. 출력 텍스처 생성
    OnResize(device, WINDOW_WIDTH, WINDOW_HEIGHT);
    return true;
}

bool Context::BuildSceneBuffers(ID3D11Device* device) {
    std::vector<Vertex>      allVertices;
    std::vector<uint32_t>    allIndices;
    std::vector<GpuMeshInfo> meshInfos;
    std::vector<GpuMaterial> materials;

    // 모델이 있으면 메시 데이터 수집
    if (m_model) {
        for (const auto& mesh : m_model->GetMeshes()) {
            // 이 메시의 오프셋 기록
            GpuMeshInfo info;
            info.vertexOffset  = (uint32_t)allVertices.size();
            info.indexOffset   = (uint32_t)allIndices.size();
            info.indexCount    = mesh->GetIndexCount();
            info.materialIndex = (uint32_t)materials.size();
            meshInfos.push_back(info);

            // 재질 추가
            const auto& mat = mesh->GetMaterial();
            GpuMaterial gpuMat;
            gpuMat.albedo    = mat.albedo;
            gpuMat.roughness = mat.roughness;
            gpuMat.emissive  = mat.emissive;
            gpuMat.metallic  = mat.metallic;
            materials.push_back(gpuMat);

            // 버텍스/인덱스 수집 (CPU 사본에서 읽기)
            uint32_t vOffset = (uint32_t)allVertices.size();
            for (const auto& v : mesh->GetVertices())
                allVertices.push_back(v);

            for (const auto& idx : mesh->GetIndices())
                allIndices.push_back(vOffset + idx); // 오프셋 보정
        }
    }

    // 모델이 없거나 비어있으면 기본 씬 (구 + 평면은 셰이더에서 하드코딩)
    if (meshInfos.empty()) {
        SPDLOG_INFO("No mesh data. Using hardcoded scene in shader.");
        // 더미 데이터 1개 (셰이더가 triCount=0을 처리함)
        allVertices.push_back(Vertex{});
        allIndices.push_back(0);
        allIndices.push_back(0);
        allIndices.push_back(0);

        GpuMeshInfo dummy{};
        dummy.vertexOffset  = 0;
        dummy.indexOffset   = 0;
        dummy.indexCount    = 0; // 0이면 셰이더가 순회 안 함
        dummy.materialIndex = 0;
        meshInfos.push_back(dummy);

        GpuMaterial dmat{};
        dmat.albedo    = glm::vec3(0.8f);
        dmat.roughness = 0.5f;
        dmat.metallic  = 0.0f;
        dmat.emissive  = glm::vec3(0.0f);
        materials.push_back(dmat);
    }

    m_meshCount = (uint32_t)meshInfos.size();

    // GPU 버퍼 생성
    m_vertexBuffer = Buffer::CreateWithData(
        device, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT,
        allVertices.data(), sizeof(Vertex), (uint32_t)allVertices.size(),
        D3D11_RESOURCE_MISC_BUFFER_STRUCTURED
    );
    if (!m_vertexBuffer) return false;
    m_vertexSRV = m_vertexBuffer->CreateSRV(device);

    m_indexBuffer = Buffer::CreateWithData(
        device, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT,
        allIndices.data(), sizeof(uint32_t), (uint32_t)allIndices.size(),
        D3D11_RESOURCE_MISC_BUFFER_STRUCTURED
    );
    if (!m_indexBuffer) return false;
    m_indexSRV = m_indexBuffer->CreateSRV(device);

    m_meshInfoBuffer = Buffer::CreateWithData(
        device, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT,
        meshInfos.data(), sizeof(GpuMeshInfo), (uint32_t)meshInfos.size(),
        D3D11_RESOURCE_MISC_BUFFER_STRUCTURED
    );
    if (!m_meshInfoBuffer) return false;
    m_meshInfoSRV = m_meshInfoBuffer->CreateSRV(device);

    m_materialBuffer = Buffer::CreateWithData(
        device, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT,
        materials.data(), sizeof(GpuMaterial), (uint32_t)materials.size(),
        D3D11_RESOURCE_MISC_BUFFER_STRUCTURED
    );
    if (!m_materialBuffer) return false;
    m_materialSRV = m_materialBuffer->CreateSRV(device);

    SPDLOG_INFO("Scene built: {} meshes, {} vertices, {} indices",
        m_meshCount, allVertices.size(), allIndices.size());
    return true;
}

void Context::OnResize(ID3D11Device* device, uint32_t width, uint32_t height) {
    m_outputUAV.Reset(); m_outputSRV.Reset(); m_outputTexture.Reset();
    m_accumUAV.Reset();  m_accumSRV.Reset();  m_accumTexture.Reset();

    // LDR 출력 텍스처 (u1)
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width; desc.Height = height;
    desc.MipLevels = 1; desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    device->CreateTexture2D(&desc, nullptr, m_outputTexture.ReleaseAndGetAddressOf());
    device->CreateUnorderedAccessView(m_outputTexture.Get(), nullptr, m_outputUAV.ReleaseAndGetAddressOf());
    device->CreateShaderResourceView(m_outputTexture.Get(), nullptr, m_outputSRV.ReleaseAndGetAddressOf());

    // HDR 누적 버퍼 (u0)
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    device->CreateTexture2D(&desc, nullptr, m_accumTexture.ReleaseAndGetAddressOf());
    device->CreateUnorderedAccessView(m_accumTexture.Get(), nullptr, m_accumUAV.ReleaseAndGetAddressOf());
    device->CreateShaderResourceView(m_accumTexture.Get(), nullptr, m_accumSRV.ReleaseAndGetAddressOf());

    m_frameCount = 0;
}

void Context::Render(ID3D11DeviceContext* context, uint32_t width, uint32_t height) {
    // 카메라 벡터 계산
    glm::vec3 front;
    front.x = cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
    front.y = sin(glm::radians(m_pitch));
    front.z = cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));
    m_cameraFront = glm::normalize(front);
    glm::vec3 right = glm::normalize(glm::cross(m_cameraFront, m_cameraUp));

    // 상수 버퍼 업데이트
    GlobalUniforms globalData;
    globalData.cameraPos   = m_cameraPos;
    globalData.fov         = glm::radians(45.0f);
    globalData.cameraFront = m_cameraFront;
    globalData.aspectRatio = (float)width / (float)height;
    globalData.cameraUp    = m_cameraUp;
    globalData.frameCount  = (float)m_frameCount++;
    globalData.cameraRight = right;
    globalData._pad        = 0.0f;
    m_globalBuffer->UpdateData(context, globalData);

    // b0: 상수 버퍼
    auto gBuf = m_globalBuffer->GetBuffer();
    context->CSSetConstantBuffers(0, 1, &gBuf);

    // t0~t3: 씬 데이터 SRV
    ID3D11ShaderResourceView* srvs[4] = {
        m_vertexSRV.Get(),    // t0: 버텍스
        m_indexSRV.Get(),     // t1: 인덱스
        m_meshInfoSRV.Get(),  // t2: 메시 정보
        m_materialSRV.Get(),  // t3: 재질
    };
    context->CSSetShaderResources(0, 4, srvs);

    // u0: HDR 누적, u1: LDR 출력
    ID3D11UnorderedAccessView* uavs[2] = {
        m_accumUAV.Get(),
        m_outputUAV.Get(),
    };
    context->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);

    // Dispatch
    uint32_t gx = (width  + 7) / 8;
    uint32_t gy = (height + 7) / 8;
    m_pathTracerProgram->Dispatch(context, gx, gy, 1);

    // 리소스 해제
    ID3D11UnorderedAccessView* nullUAVs[2] = { nullptr, nullptr };
    context->CSSetUnorderedAccessViews(0, 2, nullUAVs, nullptr);
    ID3D11ShaderResourceView* nullSRVs[4] = { nullptr, nullptr, nullptr, nullptr };
    context->CSSetShaderResources(0, 4, nullSRVs);
}

void Context::Present(ID3D11DeviceContext* context, ID3D11RenderTargetView* rtv) {
    ID3D11Resource* backBufferRes = nullptr;
    rtv->GetResource(&backBufferRes);
    if (backBufferRes) {
        context->CopyResource(backBufferRes, m_outputTexture.Get());
        backBufferRes->Release();
    }
}

void Context::ProcessMouseMenu(float dx, float dy) {
    m_yaw   += dx * m_mouseSensitivity;
    m_pitch -= dy * m_mouseSensitivity;
    m_pitch  = glm::clamp(m_pitch, -89.0f, 89.0f);
    m_frameCount = 0;
}

void Context::ProcessKeyboard(float deltaTime) {
    glm::vec3 right = glm::normalize(glm::cross(m_cameraFront, m_cameraUp));
    float speed = m_cameraSpeed * deltaTime;
    bool moved = false;

    if (GetAsyncKeyState('W') & 0x8000) { m_cameraPos += m_cameraFront * speed; moved = true; }
    if (GetAsyncKeyState('S') & 0x8000) { m_cameraPos -= m_cameraFront * speed; moved = true; }
    if (GetAsyncKeyState('A') & 0x8000) { m_cameraPos -= right          * speed; moved = true; }
    if (GetAsyncKeyState('D') & 0x8000) { m_cameraPos += right          * speed; moved = true; }
    if (GetAsyncKeyState('E') & 0x8000) { m_cameraPos += m_cameraUp     * speed; moved = true; }
    if (GetAsyncKeyState('Q') & 0x8000) { m_cameraPos -= m_cameraUp     * speed; moved = true; }

    if (moved) m_frameCount = 0;
}