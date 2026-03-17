#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "common.h"

CLASS_PTR(Buffer)
class Buffer {
public:
    static BufferUPtr CreateWithData(ID3D11Device* device, UINT bindFlags, D3D11_USAGE usage, const void* data, size_t dataSize, UINT stride);
    ~Buffer();
    
    void Bind(ID3D11DeviceContext* context, UINT slot = 0) const;

private:
    Buffer() {}
    bool Init(ID3D11Device* device, UINT bindFlags, D3D11_USAGE usage, const void* data, size_t dataSize, UINT stride);

    ComPtr<ID3D11Buffer> m_buffer;
    UINT m_bindFlags{ 0 };
    UINT m_stride{ 0 };
};

#endif // __BUFFER_H__