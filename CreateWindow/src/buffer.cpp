#include "buffer.h"

BufferUPtr Buffer::CreateWithData(ID3D11Device* device, UINT bindFlags, D3D11_USAGE usage, const void* data, size_t dataSize, UINT stride) {
    auto buffer = BufferUPtr(new Buffer());
    if (!buffer->Init(device, bindFlags, usage, data, dataSize, stride)) {
        return nullptr;
    }
    return std::move(buffer);
}

Buffer::~Buffer() {}

bool Buffer::Init(ID3D11Device* device, UINT bindFlags, D3D11_USAGE usage, const void* data, size_t dataSize, UINT stride) {
    m_bindFlags = bindFlags;
    m_stride = stride;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = usage;
    bd.ByteWidth = (UINT)dataSize;
    bd.BindFlags = bindFlags;
    bd.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;

    D3D11_SUBRESOURCE_DATA sd = {};
    if (data) {
        sd.pSysMem = data;
    }

    HRESULT hr = device->CreateBuffer(&bd, data ? &sd : nullptr, &m_buffer);
    if (FAILED(hr)) {
        SPDLOG_ERROR("Failed to create DX11 Buffer");
        return false;
    }
    return true;
}

void Buffer::Bind(ID3D11DeviceContext* context, UINT slot) const {
    UINT offset = 0;
    if (m_bindFlags & D3D11_BIND_VERTEX_BUFFER) {
        context->IASetVertexBuffers(slot, 1, m_buffer.GetAddressOf(), &m_stride, &offset);
    } else if (m_bindFlags & D3D11_BIND_INDEX_BUFFER) {
        context->IASetIndexBuffer(m_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    }
}
