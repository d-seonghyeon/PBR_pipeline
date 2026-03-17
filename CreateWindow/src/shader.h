#ifndef __SHADER_H__
#define __SHADER_H__
#include "common.h"

CLASS_PTR(Shader)
class Shader {
public:
    static ShaderUPtr CreateFromFile(const std::string& filename, const std::string& entry, const std::string& profile);
    ~Shader();
    bool LoadFile(const std::string& filename, const std::string& entry, const std::string& profile);
    ID3DBlob* GetBuffer() const { return m_shaderBlob.Get(); 
    }

private:
    Shader() {}
    ComPtr<ID3DBlob> m_shaderBlob;
};

#endif